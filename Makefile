CC = gcc
CFLAGS = -Wall -std=c11
LDFLAGS = -pthread

TARGETS = dyspozytornia magazyn

all: $(TARGETS)


dyspozytornia: dyspozytornia.o shared_memory.o
	$(CC) $(CFLAGS) -o dyspozytornia dyspozytornia.o shared_memory.o $(LDFLAGS)

magazyn: magazyn.o shared_memory.o
	$(CC) $(CFLAGS) -o magazyn magazyn.o shared_memory.o $(LDFLAGS)


dyspozytornia.o: dyspozytornia.c shared_memory.h
	$(CC) $(CFLAGS) -c dyspozytornia.c

magazyn.o: magazyn.c shared_memory.h
	$(CC) $(CFLAGS) -c magazyn.c

shared_memory.o: shared_memory.c shared_memory.h
	$(CC) $(CFLAGS) -c shared_memory.c

clean:
	rm -f *.o $(TARGETS)