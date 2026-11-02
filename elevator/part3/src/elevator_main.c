// SPDX-License-Identifier: GPL-2.0
/* Module setup and teardown, plus the implementations behind the three system calls. */

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "elevator.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("COP4610 Project 2 Group");
MODULE_DESCRIPTION("Scheduling algorithm for a pet elevator");

/* Function pointers defined in the kernel by syscalls/syscalls.c. */
extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int start_floor, int destination_floor, int type);
extern int (*STUB_stop_elevator)(void);

struct pet_elevator pet_elevator;

const int pet_weights[NUM_PET_TYPES] = {
	[PET_CHIHUAHUA] = 3,
	[PET_PUG] = 14,
	[PET_PUGHUAHUA] = 10,
	[PET_DACHSHUND] = 16,
};

const char pet_symbols[NUM_PET_TYPES] = {
	[PET_CHIHUAHUA] = 'C',
	[PET_PUG] = 'P',
	[PET_PUGHUAHUA] = 'H',
	[PET_DACHSHUND] = 'D',
};

/* Returns 1 if the elevator is already active (or still deactivating), 0 on a successful start. */
static int start_elevator_impl(void)
{
	if (mutex_lock_interruptible(&pet_elevator.lock))
		return -ERESTARTSYS;

	if (pet_elevator.state != STATE_OFFLINE) {
		mutex_unlock(&pet_elevator.lock);
		return 1;
	}

	pet_elevator.state = STATE_IDLE;
	pet_elevator.direction = DIRECTION_UP;
	pet_elevator.current_floor = MIN_FLOOR;
	pet_elevator.load_lbs = 0;
	pet_sched_notify();
	mutex_unlock(&pet_elevator.lock);
	return 0;
}

/*
 * Returns 1 if the request is invalid, 0 otherwise. Requests are accepted in every state:
 * pets wait indefinitely, so they stay on their floor until the elevator is next running.
 */
static int issue_request_impl(int start_floor, int destination_floor, int type)
{
	struct pet *pet;

	if (start_floor < MIN_FLOOR || start_floor > MAX_FLOOR ||
	    destination_floor < MIN_FLOOR || destination_floor > MAX_FLOOR ||
	    type < 0 || type >= NUM_PET_TYPES)
		return 1;

	pet = kmalloc(sizeof(*pet), GFP_KERNEL);
	if (!pet)
		return -ENOMEM;

	pet->type = type;
	pet->destination = destination_floor;

	if (mutex_lock_interruptible(&pet_elevator.lock)) {
		kfree(pet);
		return -ERESTARTSYS;
	}

	list_add_tail(&pet->node, floor_waiting_list(start_floor));
	pet_elevator.pets_waiting++;
	pet_sched_notify();
	mutex_unlock(&pet_elevator.lock);
	return 0;
}

/* Returns 1 if the elevator is already deactivating, 0 otherwise. */
static int stop_elevator_impl(void)
{
	int ret = 0;

	if (mutex_lock_interruptible(&pet_elevator.lock))
		return -ERESTARTSYS;

	if (pet_elevator.stopping) {
		ret = 1;
	} else if (pet_elevator.state != STATE_OFFLINE) {
		pet_elevator.stopping = true;
		pet_sched_notify();
	}
	mutex_unlock(&pet_elevator.lock);
	return ret;
}

static void free_pet_list(struct list_head *list)
{
	struct pet *pet, *next;

	list_for_each_entry_safe(pet, next, list, node) {
		list_del(&pet->node);
		kfree(pet);
	}
}

static int __init pet_elevator_init(void)
{
	int floor;
	int ret;

	mutex_init(&pet_elevator.lock);
	init_waitqueue_head(&pet_elevator.wake);
	INIT_LIST_HEAD(&pet_elevator.onboard);
	for (floor = MIN_FLOOR; floor <= MAX_FLOOR; floor++)
		INIT_LIST_HEAD(floor_waiting_list(floor));

	pet_elevator.state = STATE_OFFLINE;
	pet_elevator.direction = DIRECTION_UP;
	pet_elevator.current_floor = MIN_FLOOR;

	ret = pet_proc_create();
	if (ret)
		return ret;

	ret = pet_sched_start();
	if (ret) {
		pet_proc_remove();
		return ret;
	}

	/* Everything is ready, so the system calls can now reach this module. */
	STUB_start_elevator = start_elevator_impl;
	STUB_issue_request = issue_request_impl;
	STUB_stop_elevator = stop_elevator_impl;
	return 0;
}

static void __exit pet_elevator_exit(void)
{
	int floor;

	STUB_start_elevator = NULL;
	STUB_issue_request = NULL;
	STUB_stop_elevator = NULL;

	pet_proc_remove();
	pet_sched_stop();

	/* The kthread is gone, so nothing else can touch the lists. */
	free_pet_list(&pet_elevator.onboard);
	for (floor = MIN_FLOOR; floor <= MAX_FLOOR; floor++)
		free_pet_list(floor_waiting_list(floor));

	mutex_destroy(&pet_elevator.lock);
}

module_init(pet_elevator_init);
module_exit(pet_elevator_exit);
