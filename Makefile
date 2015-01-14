CC=cc
CFLAGS=-g -c -Wall
LDFLAGS=-o

all: server worker

clean:
	rm *.o server worker

server: server.o
	$(CC) $(LDFLAGS) $@ $?

worker: worker.o
	$(CC) $(LDFLAGS) $@ $?

%.o: %.c
	$(CC) $(CFLAGS) $?
