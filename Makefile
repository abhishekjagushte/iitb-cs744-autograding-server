all: server client

CC = g++ -w
CC_FLAGS = -Wno-implicit-function-declaration

PROGS_SERVER = serverFiles/utilityFiles/queue/queue.cpp serverFiles/utilityFiles/error/errors.cpp serverFiles/utilityFiles/fileshare/fileshare.cpp gradingserver.cpp serverFiles/utilityFiles/request-id/request_id.cpp
OBJS_SERVER = queue.o errors.o fileshare.o gradingserver.o request_id.o

PROGS_CLIENT = serverFiles/utilityFiles/fileshare/fileshare.cpp serverFiles/utilityFiles/error/errors.cpp gradingclient.cpp
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

