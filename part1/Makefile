CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -O0

.PHONY: all trace verify clean clean-traces

all: empty part1

empty: empty.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $<

part1: part1.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $<

# Keep both traces unfiltered so the loader's baseline calls are included.
trace: all
	LC_ALL=C strace -o empty.trace ./empty
	LC_ALL=C strace -o part1.trace ./part1
	$(MAKE) verify

verify:
	awk -f verify_traces.awk empty.trace part1.trace

# Preserve the required submission traces when removing generated binaries.
clean:
	$(RM) empty part1

clean-traces:
	$(RM) empty.trace part1.trace
