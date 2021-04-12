
#ifndef __SHM_H
#define __SHM_H

#include <stdbool.h>

#define MEMORY_PATH_LEN     1024

typedef struct
{
    char path[MEMORY_PATH_LEN]; /*!< Detailed description after the member */
    int key;                    /*!< Detailed description after the member */
    int projId;                 /*!< Detailed description after the member */
    int size;                   /*!< Detailed description after the member */
    int flags;                  /*!< Detailed description after the member */
    void *shm;                  /*!< Detailed description after the member */
    int id;                     /*!< Detailed description after the member */
} Shared_Memory_t;


bool Shared_Memory_Init(Shared_Memory_t *shm);

bool Shared_Memory_Detach(Shared_Memory_t *shm);

bool Shared_Memory_Destroy(Shared_Memory_t *shm);

#endif