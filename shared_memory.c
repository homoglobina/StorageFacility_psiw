#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/types.h>
#include "shared_memory.h"

static int get_shared_block(const char *filename, int size) {
    key_t key;

    // request a key
    // key linked to a filename so thath other programs can access the same shared memory 
    key = ftok(filename,0);
    
    if (key == -1) {
        perror("Not matching key has been provided");
        return -1;
    }

    // get shared block - create if non existent
    return shmget(key, size, 0666 |IPC_CREAT);
}

char *attach_memory_block(const char *filename, int size) {
    int shmid = get_shared_block(filename, size);
    char * result;
    
    if (shmid == -1) {
        return NULL;
    }

    // attach shared memory
    result = shmat(shmid, NULL, 0);
    if (result == (char *) -1) {
        perror("shmat");
        return NULL;
    }

    return result;
}

bool detach_memory_block(char *block) {
    return (shmdt(block) != -1);
}

bool destroy_memory_block(char *filename) {
    int shmid = get_shared_block(filename, 0);
    if (shmid == -1) {
        return NULL;
    }

    return (shmctl(shmid, IPC_RMID, NULL) != -1);
}


