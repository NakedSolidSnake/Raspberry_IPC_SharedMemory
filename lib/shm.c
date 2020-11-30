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

    else if((shm->key = ftok(shm->path, shm->projId)) == -1)
        ret = EXIT_FAILURE;
    
    else if((shm->id = shmget(shm->key, shm->size, shm->flags | IPC_CREAT)) == -1)
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