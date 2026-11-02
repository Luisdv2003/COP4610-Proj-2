// SPDX-License-Identifier: GPL-2.0
/*
 * The kthread that moves the elevator, using the LOOK algorithm: keep travelling in the
 * current direction while there is work that way, then reverse.
 *
 * The lock is never held across a delay, so /proc/elevator and issue_request() stay
 * responsive while the elevator is moving or loading. This file uses plain mutex_lock()
 * because kthreads do not receive signals, so an interruptible lock would gain nothing.
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include "elevator.h"

static struct task_struct *elevator_thread;

/* The helpers below all require pet_elevator.lock to be held. */

static bool has_pet_to_unload(void)
{
	struct pet *pet;

	list_for_each_entry(pet, &pet_elevator.onboard, node) {
		if (pet->destination == pet_elevator.current_floor)
			return true;
	}
	return false;
}

/* Boarding is FIFO: returns the pet at the front of the line if it fits, otherwise NULL. */
static struct pet *next_pet_to_board(void)
{
	struct pet *pet;

	if (pet_elevator.stopping || pet_elevator.pets_onboard >= MAX_PETS)
		return NULL;

	pet = list_first_entry_or_null(floor_waiting_list(pet_elevator.current_floor),
				       struct pet, node);
	if (pet && pet_elevator.load_lbs + pet_weights[pet->type] > MAX_WEIGHT_LBS)
		return NULL;
	return pet;
}

static bool has_work_toward(int direction)
{
	struct pet *pet;
	int floor;

	list_for_each_entry(pet, &pet_elevator.onboard, node) {
		if ((pet->destination - pet_elevator.current_floor) * direction > 0)
			return true;
	}

	if (pet_elevator.stopping)
		return false;

	for (floor = pet_elevator.current_floor + direction;
	     floor >= MIN_FLOOR && floor <= MAX_FLOOR; floor += direction) {
		if (!list_empty(floor_waiting_list(floor)))
			return true;
	}
	return false;
}

static enum elevator_state choose_next_state(void)
{
	if (pet_elevator.state == STATE_OFFLINE)
		return STATE_OFFLINE;

	if (has_pet_to_unload() || next_pet_to_board())
		return STATE_LOADING;

	if (!has_work_toward(pet_elevator.direction))
		pet_elevator.direction = -pet_elevator.direction;

	if (has_work_toward(pet_elevator.direction))
		return pet_elevator.direction == DIRECTION_UP ? STATE_UP : STATE_DOWN;

	/* Reaching here means no pet is onboard, so a stopping elevator can go offline. */
	return pet_elevator.stopping ? STATE_OFFLINE : STATE_IDLE;
}

static void unload_pets(void)
{
	struct pet *pet, *next;

	list_for_each_entry_safe(pet, next, &pet_elevator.onboard, node) {
		if (pet->destination != pet_elevator.current_floor)
			continue;
		list_del(&pet->node);
		pet_elevator.load_lbs -= pet_weights[pet->type];
		pet_elevator.pets_onboard--;
		pet_elevator.pets_serviced++;
		kfree(pet);
	}
}

static void board_pets(void)
{
	struct pet *pet;

	while ((pet = next_pet_to_board())) {
		list_move_tail(&pet->node, &pet_elevator.onboard);
		pet_elevator.load_lbs += pet_weights[pet->type];
		pet_elevator.pets_onboard++;
		pet_elevator.pets_waiting--;
	}
}

/* Performs one load or one move. Returns false if there is nothing to do right now. */
static bool run_next_step(void)
{
	enum elevator_state next;

	mutex_lock(&pet_elevator.lock);
	WRITE_ONCE(pet_elevator.work_pending, false);
	next = choose_next_state();
	pet_elevator.state = next;
	if (next == STATE_OFFLINE)
		pet_elevator.stopping = false;
	mutex_unlock(&pet_elevator.lock);

	switch (next) {
	case STATE_LOADING:
		ssleep(LOAD_DELAY_SECS);
		mutex_lock(&pet_elevator.lock);
		unload_pets();
		board_pets();
		mutex_unlock(&pet_elevator.lock);
		return true;
	case STATE_UP:
	case STATE_DOWN:
		ssleep(MOVE_DELAY_SECS);
		mutex_lock(&pet_elevator.lock);
		pet_elevator.current_floor += next == STATE_UP ? DIRECTION_UP : DIRECTION_DOWN;
		mutex_unlock(&pet_elevator.lock);
		return true;
	default:
		return false;
	}
}

static int elevator_thread_fn(void *data)
{
	while (!kthread_should_stop()) {
		if (run_next_step())
			continue;

		/* work_pending is read locklessly because the wait condition must not sleep. */
		wait_event_interruptible(pet_elevator.wake,
					 kthread_should_stop() ||
					 READ_ONCE(pet_elevator.work_pending));
	}
	return 0;
}

/* Wakes the kthread after a change that may give it work. The caller holds the lock. */
void pet_sched_notify(void)
{
	WRITE_ONCE(pet_elevator.work_pending, true);
	wake_up(&pet_elevator.wake);
}

int pet_sched_start(void)
{
	elevator_thread = kthread_run(elevator_thread_fn, NULL, "pet_elevator");
	return PTR_ERR_OR_ZERO(elevator_thread);
}

void pet_sched_stop(void)
{
	kthread_stop(elevator_thread);
}
