#include <stdio.h>
#include "shared_memory.h"
#include <semaphore.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {

    if (argc != 6) {
        printf("Niepoprawna liczba argumentow\n"); // Incorrect number of arguments
        printf("Usage %s <klucz> <liczba zamowien> <max_A_per_zam> <max_B_per_zam> <max_C_per_zam>\n", argv[0]);
        return 1;
    }
    
    const char *key = argv[1];
    int max_orders = atoi(argv[2]);
    int max_A = atoi(argv[3]);
    int max_B = atoi(argv[4]);
    int max_C = atoi(argv[5]);

    for (int i = 0; i < max_orders; i++) {
        int order_A = rand() % max_A + 1;
        int order_B = rand() % max_B + 1;
        int order_C = rand() % max_C + 1;

        printf("Zamowienie %d: %d %d %d\n", i, order_A, order_B, order_C);
    }




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


    

    // pamiec wspoldzielona
    
    char *block = attach_memory_block(key, BLOCK_SIZE);
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