// SPDX-License-Identifier: GPL-2.0
/*
 * Kernel-side system calls for the pet elevator. Copy this file to
 * [kernel_version]/syscalls/syscalls.c.
 *
 * The syscalls are built into the kernel, but the elevator lives in a loadable
 * module, so each syscall forwards through a function pointer that the module
 * sets when it loads and clears when it unloads.
 */

#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/module.h>
#include <linux/syscalls.h>

int (*STUB_start_elevator)(void);
EXPORT_SYMBOL(STUB_start_elevator);

int (*STUB_issue_request)(int start_floor, int destination_floor, int type);
EXPORT_SYMBOL(STUB_issue_request);

int (*STUB_stop_elevator)(void);
EXPORT_SYMBOL(STUB_stop_elevator);

/* Syscall 548: activate the elevator. Fails with -ENOSYS if no module is loaded. */
SYSCALL_DEFINE0(start_elevator)
{
	if (STUB_start_elevator)
		return STUB_start_elevator();
	return -ENOSYS;
}

/* Syscall 549: add a pet to the waiting list of its start floor. */
SYSCALL_DEFINE3(issue_request, int, start_floor, int, destination_floor, int, type)
{
	if (STUB_issue_request)
		return STUB_issue_request(start_floor, destination_floor, type);
	return -ENOSYS;
}

/* Syscall 550: deactivate the elevator once its current pets have been offloaded. */
SYSCALL_DEFINE0(stop_elevator)
{
	if (STUB_stop_elevator)
		return STUB_stop_elevator();
	return -ENOSYS;
}
