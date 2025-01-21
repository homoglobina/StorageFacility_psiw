#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <semaphore.h>
#include "shared_memory.h"

#define KURIER_COUNT 3

typedef struct {
    int A;
    int B;
    int C;
    int cost_A;
    int cost_B;
    int cost_C;
    int earnings;
} Magazyn;

void kurier_process(Magazyn *magazyn, char *shared_mem) {
    sem_t *sem_m = sem_open(SEM_M_NAME, 0);
    if (sem_m == SEM_FAILED) {
        perror("sem_open/sem_D");
        exit(1);
    }
    
    printf("Kurier %d gotowy, zwalniam magazynowy semafor\n", getpid());
    sem_post(sem_m);
    
    // int pid = getpid();

    sem_t *sem_d = sem_open(SEM_D_NAME, 0);
    if (sem_d == SEM_FAILED) {
        perror("sem_open/sem_D");
        exit(1);
    }

    while (1) {
        sem_wait(sem_d);
        int order[3];
        memcpy(order, shared_mem, sizeof(int) * 3);
        printf("Kurier %d otrzymal zamowienie: %d %d %d\n", getpid() ,order[0], order[1], order[2]);
        printf("%d W magazynie znajduja sie : %d A %d B %d C\n",getpid(), magazyn->A, magazyn->B, magazyn->C);  





        if (magazyn->A >= order[0] && magazyn->B >= order[1] && magazyn->C >= order[2]) {
            int cost = order[0] * magazyn->cost_A + order[1] * magazyn->cost_B + order[2] * magazyn->cost_C;
            magazyn->A -= order[0];
            magazyn->B -= order[1];
            magazyn->C -= order[2];
            magazyn->earnings += cost;
            
            order[0] = cost;
            memcpy(shared_mem, order, sizeof(int) * 3);
            sem_post(sem_d);
        } else {
            printf("Brak surowcow, kurier %d sie wylacza.\n", getpid());
            exit(0);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <config file> <shared memory key>\n", argv[0]);
        return 1;
    }

    Magazyn magazyn = {0};
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("fopen");
        return 1;
    }
    fscanf(file, "%d %d %d %d %d %d", &magazyn.A, &magazyn.B, &magazyn.C, &magazyn.cost_A, &magazyn.cost_B, &magazyn.cost_C);
    fclose(file);

    char *shared_mem = attach_memory_block(argv[2], BLOCK_SIZE);
    if (!shared_mem) {
        perror("attach_memory_block");
        return 1;
    }

    for (int i = 0; i < KURIER_COUNT; i++) {
        if (fork() == 0) {
            kurier_process(&magazyn, shared_mem);
        }
    }

    for (int i = 0; i < KURIER_COUNT; i++) {
        wait(NULL);
    }

    printf("Stan magazynu: A=%d, B=%d, C=%d\n", magazyn.A, magazyn.B, magazyn.C);
    printf("Zarobione GLD: %d\n", magazyn.earnings);

    detach_memory_block(shared_mem);
    return 0;
}
