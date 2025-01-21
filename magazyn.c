#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>   // Needed for ftruncate()
#include <sys/mman.h>   // Needed for shm_open() and mmap()
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <semaphore.h>
#include "shared_memory.h"


#define KURIER_COUNT 3
#define SHM_NAME "/shm_magazyn"  // Shared memory name for Magazyn struct

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
    int pid = getpid() % 3;

    sem_t *sem_m = sem_open(SEM_M_NAME, 0);
    if (sem_m == SEM_FAILED) {
        perror("sem_open/sem_m");
        exit(1);
    }
    
    sem_t *sem_d = sem_open(SEM_D_NAME, 0);
    if (sem_d == SEM_FAILED) {
        perror("sem_open/sem_D");
        exit(1);
    }

    sem_t *sem_iteration = sem_open("/sem_iteration", 1);
    if (sem_iteration == SEM_FAILED) {
        perror("sem_open/sem_iteration");
        exit(1);
    }
    // int i = 0;

    while (1) {

        // problemy w magazynach
        // printf("Kurier %d czeka na swoja kolej\n", pid);
        sem_wait(sem_iteration);  // Lock shared memory access
       

       // problemy pomiedzy magazymami
        // printf("Kurier %d czeka na zamowienie\n", pid);
        sem_wait(sem_d);


        int order[3];
        memcpy(order, shared_mem, sizeof(int) * 3);
        printf("%d W magazynie znajduja sie : %d A %d B %d C\n", pid, magazyn->A, magazyn->B, magazyn->C);  
        printf("Kurier %d otrzymal zamowienie: %d %d %d\n", pid, order[0], order[1], order[2]);
        

        // zamowienie 000 zabija wszystkie procesy magazynu
        if (order[0] == 0 && order[1] == 0 && order[2] == 0) {
            printf("Dyspozytornia zakończyła pracę. Kurierzy się wyłączaja.\n");
            printf("Stan magazynu: A=%d, B=%d, C=%d\n", magazyn->A, magazyn->B, magazyn->C);
            printf("Zarobione GLD: %d\n", magazyn->earnings);
            sem_post(sem_iteration);  
            kill(0, SIGTERM);  
            exit(0);
        }

        if (magazyn->A >= order[0] && magazyn->B >= order[1] && magazyn->C >= order[2]) {
            int cost = order[0] * magazyn->cost_A + order[1] * magazyn->cost_B + order[2] * magazyn->cost_C;
            printf("Kurier %d dostarcza zamowienie za %d GLD\n", pid, cost);
            magazyn->A -= order[0];
            magazyn->B -= order[1];
            magazyn->C -= order[2];
            magazyn->earnings += cost;
            printf("Kasa: %d\n", magazyn->earnings);
            
            order[0] = cost;
            memcpy(shared_mem, order, sizeof(int) * 3);
            sem_post(sem_m);
        }
        else {
            printf("Brak surowcow, kurier %d sie wylacza.\n", pid);
            sem_post(sem_iteration);  // Release before exiting
            sem_post(sem_m);
            exit(0);
        }

        sem_post(sem_iteration);  // Unlock access
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <config file> <shared memory key>\n", argv[0]);
        return 1;
    }

    // pamiec wspoldzielona dla magazynu struct
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }
    ftruncate(shm_fd, sizeof(Magazyn));  

    Magazyn *magazyn = mmap(NULL, sizeof(Magazyn), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (magazyn == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // wczytaj dane do magazyn
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("fopen");
        return 1;
    }
    fscanf(file, "%d %d %d %d %d %d", &magazyn->A, &magazyn->B, &magazyn->C, &magazyn->cost_A, &magazyn->cost_B, &magazyn->cost_C);
    fclose(file);
    magazyn->earnings = 0;

    // pamiec wspoldzielona dla magazynu
    char *shared_mem = attach_memory_block(argv[2], BLOCK_SIZE);
    if (!shared_mem) {
        perror("attach_memory_block");
        return 1;
    }


    sem_unlink("/sem_iteration");  // Usunięcie starego semafora, jeśli istnieje
    sem_t *sem_iteration = sem_open("/sem_iteration", O_CREAT | O_EXCL, 0666, 1);
    if (sem_iteration == SEM_FAILED) {
        perror("sem_open/sem_iteration");
        return 1;
    }


    printf("Magazyn  przed odbiorem zamowien: %d %d %d\n", magazyn->A, magazyn->B, magazyn->C);

    for (int i = 0; i < KURIER_COUNT; i++) {
        if (fork() == 0) {
            kurier_process(magazyn, shared_mem);
        }
    }

    for (int i = 0; i < KURIER_COUNT; i++) {
        wait(NULL);
    }

    printf("Stan magazynu: A=%d, B=%d, C=%d\n", magazyn->A, magazyn->B, magazyn->C);
    printf("Zarobione GLD: %d\n", magazyn->earnings);

    sem_close(sem_iteration);
    sem_unlink("/sem_iteration");

    munmap(magazyn, sizeof(Magazyn));  // Unmap shared memory
    shm_unlink(SHM_NAME);              // Remove shared memory
    detach_memory_block(shared_mem);
    
    return 0;
}
