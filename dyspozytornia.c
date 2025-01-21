#include <stdio.h>
#include "shared_memory.h"
#include <semaphore.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>


int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Niepoprawna liczba argumentow\n"); // Incorrect number of arguments
        return 1;
    }

    // setting up semaphores

    sem_unlink(SEM_A_NAME);
    sem_unlink(SEM_C_NAME);

    sem_t *sem_a = sem_open(SEM_A_NAME, O_CREAT, 0666, 1);
    if (sem_a == SEM_FAILED) {
        perror("sem_open/sem_a");
        return 1;
    }

    sem_t *sem_c = sem_open(SEM_C_NAME, O_CREAT, 0666, 0);
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

    while (true){
        sem_wait(sem_c);
        if (strlen(block) > 0){
            printf("Odczytano: %s\n", block);
            bool done = (strcmp(block, "quit") == 0);
            block[0] = 0;
            if (done) {break;}
        }
        sem_post(sem_a);
    }

    sem_close(sem_a);
    sem_close(sem_c);

    detach_memory_block(block);


    return 0;
}