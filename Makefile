all: server client

CC = gcc -w
CC_FLAGS = -Wno-implicit-function-declaration

PROGS_SERVER = serverFiles/utilityFiles/queue/queue.c serverFiles/utilityFiles/error/errors.c serverFiles/utilityFiles/fileshare/fileshare.c gradingserver.c
OBJS_SERVER = queue.o errors.o fileshare.o gradingserver.o

PROGS_CLIENT = serverFiles/utilityFiles/fileshare/fileshare.c serverFiles/utilityFiles/error/errors.c gradingclient.c
OBJS_CLIENT = fileshare.o errors.o gradingclient.o

OBJS_FINAL = client server


server: compile_server
	$(CC) $(CC_FLAGS) -o server $(OBJS_SERVER)

client: compile_client
	$(CC) $(CC_FLAGS) -o client $(OBJS_CLIENT)

compile_server:
	$(CC) $(CC_FLAGS) -c $(PROGS_SERVER)
	
compile_client:
	$(CC) $(CC_FLAGS) -c $(PROGS_CLIENT)

clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT) $(OBJS_FINAL)

