# Target library
lib := libuthread.a

# src
objs := queue.o uthread.o context.o preempt.o sem.o

# flags
CFLAGS	:= -Wall -Wextra -Werror
CFLAGS	+= -pipe
## Debug flag
ifneq ($(D),1)
CFLAGS	+= -O2
else
CFLAGS	+= -g
endif

all: $(lib)

$(lib): $(objs)
	ar rcs $@ $^

%.o: %.c
	gcc $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -rf $(objs)

.PHONY: globber
globber:
	rm -rf $(objs) $(lib)