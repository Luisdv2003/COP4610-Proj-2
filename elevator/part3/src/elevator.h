/* SPDX-License-Identifier: GPL-2.0 */
/* Types and shared state for the pet elevator module. */

#ifndef PET_ELEVATOR_H
#define PET_ELEVATOR_H

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/wait.h>

#define MIN_FLOOR 1
#define MAX_FLOOR 5
#define NUM_FLOORS (MAX_FLOOR - MIN_FLOOR + 1)

#define MAX_PETS 5
#define MAX_WEIGHT_LBS 50

#define MOVE_DELAY_SECS 2
#define LOAD_DELAY_SECS 1

#define DIRECTION_UP 1
#define DIRECTION_DOWN (-1)

enum pet_type {
	PET_CHIHUAHUA,
	PET_PUG,
	PET_PUGHUAHUA,
	PET_DACHSHUND,
	NUM_PET_TYPES,
};

enum elevator_state {
	STATE_OFFLINE,
	STATE_IDLE,
	STATE_LOADING,
	STATE_UP,
	STATE_DOWN,
};

/* A pet waiting on a floor or riding the elevator; allocated with kmalloc. */
struct pet {
	int type;
	int destination;
	struct list_head node;
};

/* All fields below the lock are protected by it, except as noted on work_pending. */
struct pet_elevator {
	struct mutex lock;
	enum elevator_state state;
	int direction;
	int current_floor;
	int load_lbs;
	int pets_onboard;
	int pets_waiting;
	int pets_serviced;
	/* stop_elevator() was called; offloading pets before going OFFLINE. */
	bool stopping;
	/* Written under the lock, read locklessly by the waiting kthread. */
	bool work_pending;
	struct list_head onboard;
	/* FIFO waiting list per floor, lobby first. */
	struct list_head floors[NUM_FLOORS];
	/* The kthread sleeps here while it has no work. */
	wait_queue_head_t wake;
};

extern struct pet_elevator pet_elevator;
extern const int pet_weights[NUM_PET_TYPES];
extern const char pet_symbols[NUM_PET_TYPES];

static inline struct list_head *floor_waiting_list(int floor)
{
	return &pet_elevator.floors[floor - MIN_FLOOR];
}

/* elevator_sched.c: the kthread that moves the elevator. */
int pet_sched_start(void);
void pet_sched_stop(void);
void pet_sched_notify(void);

/* elevator_proc.c: the /proc/elevator entry. */
int pet_proc_create(void);
void pet_proc_remove(void);

#endif /* PET_ELEVATOR_H */
