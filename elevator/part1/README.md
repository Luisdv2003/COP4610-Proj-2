# Part 1: System Call Tracing

COP4610 Project 2  
Author: Juan Ignacio Medina

This part compares the system calls made by an empty C program with those made
by a program containing five additional system calls.

## Files

| File | Description |
| --- | --- |
| `empty.c` | Baseline program that returns 0. |
| `part1.c` | Baseline program with five explicit system calls added. |
| `empty.trace` | Complete `strace` output for the baseline program. |
| `part1.trace` | Complete `strace` output for the modified program. |
| `Makefile` | Builds the programs, generates traces, and runs verification. |
| `verify_traces.awk` | Checks the syscall counts and successful program exits. |

## System calls

`part1.c` uses `syscall()` to invoke each of these calls once:

| System call | Return value |
| --- | --- |
| `getpid` | Current process ID. |
| `getppid` | Parent process ID. |
| `getuid` | Real user ID. |
| `geteuid` | Effective user ID. |
| `getgid` | Real group ID. |

Neither program prints output. The baseline trace includes the calls made during
program startup and termination.

## Build and run

Use Linux or WSL with GCC, GNU Make, `strace`, and `awk` installed. From `part1/`:

```sh
make trace
```

This builds both programs, saves their complete traces, and runs the verifier.
To check the saved traces without rebuilding or running the programs again:

```sh
make verify
```

## Results

The included traces were generated on Ubuntu under WSL:

```text
empty: 31 calls; part1: 36 calls; difference: 5
PASS: exactly one extra getpid, getppid, getuid, geteuid, and getgid.
```

Both programs exited with status 0. The verifier checks that each of the five
selected calls occurs once more than in the baseline, and that all other syscall
counts match. Startup call counts can differ between systems; the expected
difference is five.

## Clean up

```sh
make clean
```

This removes the executables and keeps both trace files.
