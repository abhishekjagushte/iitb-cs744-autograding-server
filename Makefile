all: server client

CC = g++ -w

# Server files
PROGS_SERVER = serverFiles/utilityFiles/queue/queue.cpp serverFiles/utilityFiles/error/errors.cpp serverFiles/utilityFiles/fileshare/fileshare.cpp gradingserver.cpp serverFiles/utilityFiles/request-id/request_id.cpp

OBJS_SERVER = queue.o errors.o fileshare.o gradingserver.o request_id.o

# Client files
PROGS_CLIENT = serverFiles/utilityFiles/fileshare/fileshare.cpp serverFiles/utilityFiles/error/errors.cpp gradingclient.cpp

OBJS_CLIENT = fileshare.o errors.o gradingclient.o


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

