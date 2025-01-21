#include <stdio.h>
#include "shared_memory.h"
#include <semaphore.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <mx_conf.txt> <key>\n", argv[0]);
        return 1;
    }
    
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    const char *key = argv[2];

    
    int order_A = 0;
    int order_B = 0;
    int order_C = 0;
    int amount_A = 0;
    int amount_B = 0;
    int amount_C = 0;
    int price_A = 0;
    int price_B = 0;
    int price_C = 0;


    // Czytanie i zapisanie do zmiennych
    
    char buffer[64];
    read(fd, buffer, 64);   
    sscanf(buffer, "%d %d %d %d %d %d", &amount_A, &amount_B, &amount_C , &price_A, &price_B, &price_C);        // to change
    printf("Amount A: %d\nAmount B: %d\nAmount C: %d\n", amount_A, amount_B, amount_C);
    printf("Price A: %d\nPrice B: %d\nPrice C: %d\n", price_A, price_B, price_C);
    close(fd);
    


    
    // Tworzenie procesow Kurierow 
    int pid;
    if ((pid = fork()) == 0){   // 3 procesy kurier dziadek -> ojciec -> dziecko
        pid = fork();
    }


    // komunikacja magazyn - kurier fifo kolejki
    if((fd = mkfifo("magazyn", O_WRONLY)) == -1){

        perror("mkfifo");
        return 1;
    }





    int cost;
    // emulacja dyspozytorni
    while (amount_A > 0 && amount_B > 0 && amount_C > 0){
        printf("PID: %d\nile A B C:", pid);
        scanf("%d %d %d", &order_A, &order_B, &order_C);
        if (order_A > amount_A){
            printf("Za duzo A\n");
            continue;
        }
        if (order_B > amount_B){
            printf("Za duzo B\n");
            continue;

        }
        if (order_C > amount_C){
            printf("Za duzo C\n");

            continue;
        }
        
        amount_A -= order_A;
        amount_B -= order_B;
        amount_C -= order_C;

        cost = order_A * price_A + order_B * price_B + order_C * price_C;

        printf("Zamowienie kosztowalo: %d\n", cost);

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
    char *block = attach_memory_block(key, BLOCK_SIZE);
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