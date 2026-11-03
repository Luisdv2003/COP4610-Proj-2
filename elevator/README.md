# Elevator

[Description]

## Group Members
- **Luis Dominguez**
- **Pragun Kapur**
- **Juan Medina Molina**
## Division of Labor

### Part 1: System Call Tracing
- **Responsibilities**: [Description]
- **Assigned to**: Juan Medina Molina

### Part 2: Timer Kernel Module
- **Responsibilities**: [Description]
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
For a C/C++ example:
```bash
make
```
This will build the executable in ...
### Execution
```bash
make run
```
This will run the program ...

## Part 2

### Compilation
For a C/C++ example:
```bash
make
```
This will build the executable in ...
### Execution
```bash
make run
```
This will run the program ...


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
| 2026-11-01 | With AI assistance (Claude), wrote the syscalls, elevator module, scheduler, proc output and Makefile. |
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
Document in-person meetings, their purpose, and what was discussed.

| Date       | Attendees            | Topics Discussed | Outcomes / Decisions |
|------------|----------------------|------------------|-----------------------|
| YYYY-MM-DD | [Names]              | [Agenda items]   | [Actions/Next steps]  |
| YYYY-MM-DD | [Names]              | [Agenda items]   | [Actions/Next steps]  |
| YYYY-MM-DD | [Names]              | [Agenda items]   | [Actions/Next steps]  |



## Bugs
- **Part 3:** A syscall in flight at the exact moment of `rmmod` could call into unloaded code, because the kernel hooks are not reference counted. Stop the elevator and let `producer` finish before unloading.

## Considerations
- **Part 3:** The starter `producer.c` picks floors from 1 to 6, so its floor 6 requests return 1 by design.
- **Part 3:** Pets can queue while the elevator is `OFFLINE`, as the Elevator slides show.
- **Part 3:** `start_elevator()` returns 1 while the elevator is still deactivating.
