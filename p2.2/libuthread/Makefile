# Target library
lib := libuthread.a

# Object files for Phase 1
objs := queue.o

# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -Werror -g -std=c99

all: $(lib)

## TODO: Phase 1

$(lib): $(objs)
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(objs) $(lib)
