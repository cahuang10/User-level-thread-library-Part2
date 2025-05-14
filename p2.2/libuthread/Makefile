# Target library
lib := libuthread.a

objs := queue.o uthread.o context.o preempt.o

# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -Werror -g -O0 -std=c99

.PHONY: all clean

all: $(lib)

$(lib): $(objs)
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(objs) $(lib)
