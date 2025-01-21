#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdbool.h>


char *attach_memory_block(const char *filename, int size);
bool detach_memory_block(char *block);
bool destroy_memory_block(char *filename);


#define SEM_A_NAME "/semaphoreA"
#define SEM_C_NAME "/semaphoreC"
#define BLOCK_SIZE 1024


#endif //SHARED_MEMORY_H