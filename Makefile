all: test

OBJS = queue.o test.o
PROGS = queue.c test.c

test: compile
	gcc -o test $(OBJS)

compile:
	gcc -c $(PROGS)

clean:
	rm -f $(OBJS)

