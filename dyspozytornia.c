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
        printf("Niepoprawna liczba argumentow\n"); /
        printf("Usage %s <klucz> <liczba zamowien> <max_A_per_zam> <max_B_per_zam> <max_C_per_zam>\n", argv[0]);
        return 1;
    }

    // zmienne
    const char *key = argv[1];
    int max_orders = atoi(argv[2]);
    int max_A = atoi(argv[3]);
    int max_B = atoi(argv[4]);
    int max_C = atoi(argv[5]);


    // Sprawdzenie i utworzenie pliku klucz, jeśli nie istnieje
    FILE *key_file = fopen(key, "a");
    if (!key_file) {
        perror("Nie można utworzyć pliku klucz\n"); // Cannot create file 
        return 1;
    }
    fclose(key_file);


    // przygotowanie semaforow , dyspozytorania musi byc pierwsza odpalona
    sem_unlink(SEM_D_NAME);
    sem_unlink(SEM_M_NAME);

    sem_t *sem_d = sem_open(SEM_D_NAME, O_CREAT, 0666, 0);
    if (sem_d == SEM_FAILED) {
        perror("sem_open/sem_D");
        return 1;
    }

    sem_t *sem_m = sem_open(SEM_M_NAME, O_CREAT, 0666, 1);
    if (sem_m == SEM_FAILED) {
        perror("sem_open/sem_M");
        return 1;
    }

    // przygotowanie pamiecie wspoldzielonej w ktorej zapisywane beda zamowienia
    char *block = attach_memory_block(key, BLOCK_SIZE);
    if (block == NULL) {
        perror("attach_memory_block");
        return 1;
    }



    int arr[3];
    int GLD = 0;


    printf("Dyspozytornia przygotowuje zamowienia\n");

    for (int i = 0; i < max_orders; i++) {
        printf("Czekamy na sygnal magazynowy %d\n", i);
        sem_wait(sem_m);  // czeka na sygnal magazynowy
        printf("Dostarczono sygnal magazynowy %d\n", i);
        
        
        GLD += arr[0];

        arr[0] = rand() % max_A + 1;
        arr[1] = rand() % max_B + 1;
        arr[2] = rand() % max_C + 1;

        printf("Przekazywanie zamowienia %d: %d %d %d\n", i, arr[0], arr[1], arr[2]);

        memset(block, 0, BLOCK_SIZE);
        memcpy(block, arr, sizeof(int)*3);

        sem_post(sem_d);  // przesylamy sygnal z dyspozytorni o przekazaniu zamowienia

    }


    // dystpozytornia konczy prace 
    int exit_signal[3] = {0, 0, 0};
    memcpy(block, exit_signal, sizeof(int) * 3);
    sem_post(sem_d);  



    sem_close(sem_d);
    sem_close(sem_m);

    detach_memory_block(block);

    printf("Dyspozytornia zaplacila lacznie: %d", GLD);

    return 0;
}
