<p align="center">
  <img src="https://cdn.app.compendium.com/uploads/user/e7c690e8-6ff9-102a-ac6d-e4aebca50425/bbeb190a-b93b-4d7b-bd6c-3f9928cd87d2/Image/0ff62842b17a46978cd5cee1572e0fdb/shared_memory.png">
</p>

# _Shared Memory_
## Introdução
## Implementação
### Biblioteca
#### shm.h
```c

#ifndef __SHM_H
#define __SHM_H

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
}SHMemory_t;

/**
 * @brief Inicia a Shared memory
 * 
 * @param shm Estrutura de configuração do objeto shm
 * @return int 0->Success 1->Error
 */
int SHMemory_init(SHMemory_t *shm);

/**
 * @brief Desacopla Shared Memory 
 * 
 * @param shm  Estrutura de configuração do objeto shm
 * @return int 0->Success 1->Error
 */
int SHMemory_detach(SHMemory_t *shm);

/**
 * @brief Remove Shared Memory alocada pela função  SHMemory_init
 * 
 * @param shm Estrutura de configuração do objeto shm
 * @return int 0->Success 1->Error
 */
int SHMemory_destroy(SHMemory_t *shm);

#endif
```
#### shm.c
```c
#include <shm.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>

static int validParams(SHMemory_t *shm);

int SHMemory_init(SHMemory_t *shm)
{
    int ret = EXIT_SUCCESS;

    if(validParams(shm))
        ret = EXIT_FAILURE;

    else if((shm->id = shmget(shm->projId, shm->size, shm->flags | IPC_CREAT)) == -1)
        ret = EXIT_FAILURE;

    else if((shm->shm = shmat(shm->id, (void *)0, 0)) == NULL)
        ret = EXIT_FAILURE;

    return ret;
}

int SHMemory_detach(SHMemory_t *shm)
{
    int ret = EXIT_SUCCESS;
    if (!shm)
        ret = EXIT_FAILURE;
    else
        shmdt(shm->shm);

    return ret;
}

int SHMemory_destroy(SHMemory_t *shm)
{
    int ret = EXIT_SUCCESS;
    if (!shm)
        ret = EXIT_FAILURE;
    else
        shmctl(shm->id, IPC_RMID, 0);

    return ret;
}

static int validParams(SHMemory_t *shm)
{
    int ret = EXIT_SUCCESS;
    if (!shm)
        ret = EXIT_FAILURE;

    else if(strlen(shm->path) <= 0 || strlen(shm->path) > MEMORY_PATH_LEN)
        ret = EXIT_FAILURE;

    else if(shm->size <= 0 || shm->size > MEMORY_PATH_LEN)
        ret = EXIT_FAILURE;

    return ret;
}
```


## Conclusão
