all: server

OBJS = queue.o gradingserver.o
PROGS = queue.c gradingserver.c

server: compile
	gcc -o server $(OBJS)

compile:
	gcc -c $(PROGS)

clean:
	rm -f $(OBJS)

