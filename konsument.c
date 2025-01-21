#include <stdio.h>
#include "shared_memory.h"
#include <semaphore.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Niepoprawna liczba argumentow\n");
        return 1;
    }



    // nie tworza nigdy wiec dyspozytornia musi isc pierwsza
    sem_t *sem_a = sem_open(SEM_A_NAME,0);
    if (sem_a == SEM_FAILED) {
        perror("sem_open/sem_a");
        return 1;
    }

    sem_t *sem_c = sem_open(SEM_C_NAME, 0);
    if (sem_c == SEM_FAILED) {
        perror("sem_open/sem_c");
        return 1;
    }


    // setting up shared memory
    char *block = attach_memory_block(FILENAME, BLOCK_SIZE);
    if (block == NULL) {
        perror("attach_memory_block");
        return 1;
    }


    for (int i = 0; i < 10; i++) {
        sem_wait(sem_a);  // wait for the signal
        printf("Konsument: %s\n", argv[1]);
        strncpy(block, argv[1], BLOCK_SIZE -1 );
        block[BLOCK_SIZE - 1] = 0;
        sem_post(sem_c);  // send the signal
    }



    sem_close(sem_a);
    sem_close(sem_c);

    detach_memory_block(block);


    return 0;
}