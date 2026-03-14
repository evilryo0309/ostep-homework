# Specify compiler
CC = gcc

# Specify compilation options
# -Wall: Show all warning messages
# -O0: Disable optimization to ensure measurement loops are not optimized away
CFLAGS = -Wall -O0

# Define the names of the final executables
TARGETS = measure1_syscall measure2_ctx_switch measure3_rdtsc

# Default target: executed when only 'make' is entered
all: $(TARGETS)

# Compilation rules for each program
measure1_syscall: measure1_syscall.c
	$(CC) $(CFLAGS) -o $@ $<

measure2_ctx_switch: measure2_ctx_switch.c
	$(CC) $(CFLAGS) -o $@ $<

measure3_rdtsc: measure3_rdtsc.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean command: used to delete compiled executables and keep the directory clean
clean:
	rm -f $(TARGETS)

# Declare that all and clean are not real file names, but action commands
.PHONY: all clean