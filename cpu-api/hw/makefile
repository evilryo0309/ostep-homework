# 1. Define variables (Variables)
CC = gcc
# -Wall: Show all warnings (strongly recommended to enable)
# -g: Add debug information (useful for debugging with gdb in the future)
CFLAGS = -Wall -g

# Define all target executable names you want to compile
TARGETS = q1 q2 q3 q4 q5 q6 q7 q8

# 2. Default target
# When you just type `make`, this target will be executed by default, which means compiling all TARGETS
all: $(TARGETS)

# 3. Pattern rule
# This is a magic rule that tells make: "If you want to generate any file without an extension (%), look for the corresponding .c file (%.c)"
%: %.c
	$(CC) $(CFLAGS) -o $@ $<

# 4. Clean command
# Used to remove compiled executables and keep the folder clean
clean:
	rm -f $(TARGETS)