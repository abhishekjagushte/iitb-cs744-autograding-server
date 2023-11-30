all: server client

CC = gcc -w

# Server files
PROGS_SERVER = queue.c gradingserver.c

OBJS_SERVER = queue.o gradingserver.o

# Client files
PROGS_CLIENT = gradingclient.c

OBJS_CLIENT = gradingclient.o


# Final executables
OBJS_FINAL = client server


server: compile_server
	$(CC) -o server $(OBJS_SERVER)

client: compile_client
	$(CC) -o client $(OBJS_CLIENT)

compile_server:
	$(CC) -c $(PROGS_SERVER)
	
compile_client:
	$(CC) -c $(PROGS_CLIENT)

clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT) $(OBJS_FINAL)