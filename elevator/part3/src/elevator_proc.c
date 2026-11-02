// SPDX-License-Identifier: GPL-2.0
/* The /proc/elevator entry. */

#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "elevator.h"

#define PROC_NAME "elevator"

static const char * const state_names[] = {
	[STATE_OFFLINE] = "OFFLINE",
	[STATE_IDLE] = "IDLE",
	[STATE_LOADING] = "LOADING",
	[STATE_UP] = "UP",
	[STATE_DOWN] = "DOWN",
};

static void show_pet_list(struct seq_file *m, struct list_head *list)
{
	struct pet *pet;

	list_for_each_entry(pet, list, node)
		seq_printf(m, " %c%d", pet_symbols[pet->type], pet->destination);
}

/* Returns how many pets are on the given floor's waiting list. Requires the lock. */
static int count_waiting(int floor)
{
	struct list_head *pos;
	int count = 0;

	list_for_each(pos, floor_waiting_list(floor))
		count++;
	return count;
}

static int elevator_proc_show(struct seq_file *m, void *v)
{
	int floor;

	if (mutex_lock_interruptible(&pet_elevator.lock))
		return -ERESTARTSYS;

	seq_printf(m, "Elevator state: %s\n", state_names[pet_elevator.state]);
	seq_printf(m, "Current floor: %d\n", pet_elevator.current_floor);
	seq_printf(m, "Current load: %d lbs\n", pet_elevator.load_lbs);
	seq_puts(m, "Elevator status:");
	show_pet_list(m, &pet_elevator.onboard);
	seq_puts(m, "\n\n");

	for (floor = MAX_FLOOR; floor >= MIN_FLOOR; floor--) {
		seq_printf(m, "[%c] Floor %d: %d", floor == pet_elevator.current_floor ? '*' : ' ',
			   floor, count_waiting(floor));
		show_pet_list(m, floor_waiting_list(floor));
		seq_putc(m, '\n');
	}

	seq_printf(m, "\nNumber of pets: %d\n", pet_elevator.pets_onboard);
	seq_printf(m, "Number of pets waiting: %d\n", pet_elevator.pets_waiting);
	seq_printf(m, "Number of pets serviced: %d\n", pet_elevator.pets_serviced);

	mutex_unlock(&pet_elevator.lock);
	return 0;
}

static int elevator_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, elevator_proc_show, NULL);
}

static const struct proc_ops elevator_proc_ops = {
	.proc_open = elevator_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int pet_proc_create(void)
{
	if (!proc_create(PROC_NAME, 0444, NULL, &elevator_proc_ops))
		return -ENOMEM;
	return 0;
}

void pet_proc_remove(void)
{
	remove_proc_entry(PROC_NAME, NULL);
}
