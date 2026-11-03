# Elevator

This project extends the Linux kernel with a scheduling algorithm for a pet elevator. Part 1 traces system calls with `strace`. Part 2 adds a kernel module that reports the time through `/proc/timer`. Part 3 adds three system calls and a kernel module that runs the elevator, with its live state shown in `/proc/elevator`.

## Group Members
- **Luis Dominguez**
- **Pragun Kapur**
- **Juan Medina Molina**
## Division of Labor

### Part 1: System Call Tracing
- **Responsibilities**: Compare the `strace` output of an empty C program with a program that makes five extra system calls (`getpid`, `getppid`, `getuid`, `geteuid`, `getgid`), and verify the difference.
- **Assigned to**: Juan Medina Molina

### Part 2: Timer Kernel Module
- **Responsibilities**: Write the `my_timer` kernel module, which creates `/proc/timer` and uses `ktime_get_real_ts64()` to report the current time and the time elapsed since the last read.
- **Assigned to**: Luis Dominguez

### Part 3a: Adding System Calls
- **Responsibilities**: Add syscalls 548-550 to the kernel using function-pointer stubs.
- **Assigned to**: Pragun Kapur

### Part 3b: Kernel Compilation
- **Responsibilities**: Build, install and boot kernel 6.16.12 with the new syscalls.
- **Assigned to**: Pragun Kapur

### Part 3c: Threads
- **Responsibilities**: Run the elevator in a kthread that sleeps on a wait queue when idle.
- **Assigned to**: Pragun Kapur

### Part 3d: Linked List
- **Responsibilities**: Store waiting pets and riders in linux/list.h lists, allocated with kmalloc.
- **Assigned to**: Pragun Kapur

### Part 3e: Mutexes
- **Responsibilities**: Protect state shared by the syscalls, kthread and proc reads with one mutex.
- **Assigned to**: Pragun Kapur

### Part 3f: Scheduling Algorithm
- **Responsibilities**: Implement LOOK scheduling with FIFO boarding and the weight and count limits.
- **Assigned to**: Pragun Kapur

## File Listing
```
elevator/
├── Makefile
├── part1/
│   ├── empty.c
│   ├── empty.trace
│   ├── part1.c
│   ├── part1.trace
│   └── Makefile
├── part2/
│   ├── src/
│   └── Makefile
├── part3/
│   ├── src/
│   ├── tests/
│   ├── Makefile
│   └── syscalls.c
├── Makefile
└── README.md

```
# How to Compile & Execute

### Requirements
- **Compiler**: e.g., `gcc` for C/C++, `rustc` for Rust.
- **Dependencies**: List any libraries or frameworks necessary (rust only).

## Part 1

### Compilation
```bash
cd part1
make
```
This will build the `empty` and `part1` executables in `part1/`.
### Execution
```bash
make trace
```
This will run both programs under `strace`, save `empty.trace` and `part1.trace`, and verify that `part1` makes exactly five more system calls. It requires `strace`. To check the saved traces without regenerating them, run `make verify`.

## Part 2

### Compilation
```bash
cd part2
make
```
This will build `my_timer.ko` in `part2/`.
### Execution
```bash
sudo insmod my_timer.ko
cat /proc/timer
sleep 1
cat /proc/timer
sudo rmmod my_timer.ko
```
This will load the module and read `/proc/timer` twice. The first read shows the current time, and later reads also show the elapsed time since the previous read. Removing the module also removes `/proc/timer`.


## Part 3

### Compilation
Kernel setup steps are in `part3/README.md`. Then build the module:
```bash
cd part3
make
```
This will build `elevator.ko` in `part3/`.
### Execution
Load the module and watch the elevator:
```bash
sudo insmod elevator.ko
watch -n 1 cat /proc/elevator
```
In another terminal, run the starter test programs:
```bash
cd part3/tests/elevator-test
make
./consumer --start
./producer 20
./consumer --stop
```
To remove the module, run this from `part3/`:
```bash
sudo rmmod elevator.ko
```

## Development Log
Each member records their contributions here.

### Luis Dominguez

| Date       | Work Completed / Notes |
|------------|------------------------|
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |

### Pragun Kapur

| Date       | Work Completed / Notes |
|------------|------------------------|
| 2026-11-01 | Wrote the syscalls, elevator module, scheduler, proc output and Makefile. |
| 2026-11-01 | Built kernel 6.16.12 and tested the module on a debug kernel with lockdep and KASAN. |
| 2026-11-01 | Rehearsed the full install and demo on an Ubuntu 24.04 VM and captured evidence. |
| 2026-11-02 | Uploaded Part 3, tested a fresh clone, and fixed the Part 2 build and compile errors. |


### Juan Medina Molina

| Date       | Work Completed / Notes |
|------------|------------------------|
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |


## Meetings
The group coordinated through online meetings and discussions.

## Bugs
- **Part 3:** A syscall in flight at the exact moment of `rmmod` could call into unloaded code, because the kernel hooks are not reference counted. Stop the elevator and let `producer` finish before unloading.

## Considerations
- **Part 3:** The starter `producer.c` picks floors from 1 to 6, so its floor 6 requests return 1 by design.
- **Part 3:** Pets can queue while the elevator is `OFFLINE`, as the Elevator slides show.
- **Part 3:** `start_elevator()` returns 1 while the elevator is still deactivating.
