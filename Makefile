CC = gcc
CFLAGS = -Wall -std=c11
LDFLAGS = -pthread

TARGETS = konsument dyspozytornia

all: $(TARGETS)

konsument: konsument.o shared_memory.o
	$(CC) $(CFLAGS) -o konsument konsument.o shared_memory.o $(LDFLAGS)

dyspozytornia: dyspozytornia.o shared_memory.o
	$(CC) $(CFLAGS) -o dyspozytornia dyspozytornia.o shared_memory.o $(LDFLAGS)

konsument.o: konsument.c shared_memory.h
	$(CC) $(CFLAGS) -c konsument.c

dyspozytornia.o: dyspozytornia.c shared_memory.h
	$(CC) $(CFLAGS) -c dyspozytornia.c

shared_memory.o: shared_memory.c shared_memory.h
	$(CC) $(CFLAGS) -c shared_memory.c

clean:
	rm -f *.o $(TARGETS)