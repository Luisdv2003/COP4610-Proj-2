# Part 3: Pet Elevator

Merge the relevant parts of this file into the group's root `README.md`.

## Files

| File | Purpose |
| --- | --- |
| `syscalls.c` | Kernel-side syscalls 548-550; copy to `[kernel_version]/syscalls/syscalls.c` |
| `Makefile` | Builds `elevator.ko` in this directory |
| `src/elevator.h` | Constants, types and shared state |
| `src/elevator_main.c` | Module init/exit and the logic behind the three syscalls |
| `src/elevator_sched.c` | kthread and LOOK scheduling algorithm |
| `src/elevator_proc.c` | `/proc/elevator` output |
| `tests/` | Starter test programs, unchanged |

## Add the system calls to the kernel

Download the latest 6.16.x kernel, which was 6.16.12 when this was tested, into `/usr/src` and
link it from your home directory. `/usr/src` is owned by root, so take ownership of the tree to
edit it without `sudo`.

```bash
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.16.12.tar.xz
sudo tar -xf linux-6.16.12.tar.xz -C /usr/src
sudo chown -R $USER /usr/src/linux-6.16.12
ln -s /usr/src/linux-6.16.12 ~/test-kernel
cd ~/test-kernel
```

Run the commands below from that directory, replacing `$REPO` with this repository. The repository
path must not contain spaces.

```bash
mkdir syscalls
cp $REPO/part3/syscalls.c syscalls/syscalls.c
echo 'obj-y := syscalls.o' > syscalls/Makefile
```

The table is at `arch/x86/entry/syscalls/syscall_64.tbl`, as the slides say. The
`arch/x86/syscalls/` path in the assignment does not exist in 6.16. Append these lines to the end
of the table:

```
548	common	start_elevator		sys_start_elevator
549	common	issue_request		sys_issue_request
550	common	stop_elevator		sys_stop_elevator
```

In `include/linux/syscalls.h`, add these lines just before the final `#endif`:

```c
asmlinkage long sys_start_elevator(void);
asmlinkage long sys_issue_request(int start_floor, int destination_floor, int type);
asmlinkage long sys_stop_elevator(void);
```

In the top-level `Makefile`, change the empty `core-y := ` line to:

```make
core-y		:= syscalls/
```

Install the build dependencies (the lab VMs already have them):

```bash
sudo apt install build-essential libncurses-dev libssl-dev libelf-dev bison flex -y
```

Optionally, limit the build to the modules this VM has loaded. The slides don't include this step,
but it cut the build to about 11 minutes on 8 CPUs, where a full build can take hours:

```bash
yes '' | make localmodconfig
```

Configure the kernel. In `menuconfig`, change nothing, then save and exit. If `make` stops to ask
about certificates, press Enter to accept the default.

```bash
make menuconfig
scripts/config --disable SYSTEM_REVOCATION_KEYS
scripts/config --disable SYSTEM_TRUSTED_KEYS
make -j$(nproc)
```

Install it and reboot:

```bash
sudo make headers_install -j$(nproc)
sudo make modules_install -j$(nproc)
sudo make install -j$(nproc)
sudo update-grub
sudo reboot
```

After rebooting, `uname -r` should show 6.16.x without `-generic` at the end.

## Check the system calls

```bash
cd part3/tests/system-calls-test
make
sudo insmod syscheck.ko
make run
sudo rmmod syscheck
```

The assignment calls this test `syscalls.ko` and `test_syscalls`, but the starter files name them
`syscheck.ko` and `test-syscalls`. Unload `syscheck` before loading `elevator.ko`, because both
set the same kernel hooks. If `insmod` reports `Key was rejected`, disable Secure Boot in the VM.

## Build and run

```bash
cd part3
make
sudo insmod elevator.ko
```

In a second terminal, watch the elevator:

```bash
watch -n 1 cat /proc/elevator
```

In the first terminal, run the starter test programs:

```bash
cd tests/elevator-test
make
./consumer --start
./producer 20
./consumer --stop
cd ../..
sudo rmmod elevator
```

The starter `producer.c` picks floors with `rnd(1, 6)`, so some requests use floor 6. The building
only has floors 1-5, so `issue_request()` correctly rejects those and `producer` prints
`returned 1` for them. `./producer 20` therefore creates fewer than 20 pets, which is expected.

`run_tests.sh` in the Example Codes zip (`part3_test_case/syscall-tests/`) runs a longer sequence
of starts, stops and new pets, which suits the demo. Its `consumer --start` can print `returned 1`
if the previous stop is still offloading pets; that start is ignored and the elevator goes offline.

## Design

- **Scheduling:** LOOK. The elevator continues in its current direction while a rider's
  destination or a waiting pet lies that way, then reverses. It stops at a floor when a pet can
  get off or the pet at the front of the line fits. It moves one floor at a time, so it never
  jumps between floors 1 and 5.
- **FIFO boarding:** boarding stops at the first pet that does not fit, so no pet is overtaken.
  Every rider is delivered within a sweep, so space always frees up and no pet waits forever.
- **Locking:** one mutex protects all shared state. It is released during the 1 s and 2 s delays,
  so `/proc/elevator` and `issue_request()` never block for the length of a delay. The syscalls
  and proc read use `mutex_lock_interruptible`; the kthread uses `mutex_lock` because kthreads do
  not receive signals.
- **kthread:** created at `insmod` and stopped at `rmmod`. While it has no work it sleeps on a
  wait queue instead of polling, and the syscalls wake it.
- **Memory:** pets are allocated with `kmalloc` in `issue_request()` and freed when they get off.
  Any left over are freed at `rmmod`.

## Behaviour the assignment leaves open

- `issue_request()` accepts pets in every state. The Elevator slides show pets queuing while
  `OFFLINE`, and they are serviced the next time the elevator runs.
- `start_elevator()` returns 1 while the elevator is still deactivating.
- `stop_elevator()` returns 0 and changes nothing if the elevator is already `OFFLINE`.
- The serviced count covers the life of the module, so it is not reset by `start_elevator()`.
- `issue_request()` returns `-ENOMEM` if `kmalloc` fails.

## Known limitation

The kernel hooks are not reference counted, so a syscall in flight at the exact moment of `rmmod`
could call into unloaded code. Stop the elevator and let `producer` finish before unloading.
