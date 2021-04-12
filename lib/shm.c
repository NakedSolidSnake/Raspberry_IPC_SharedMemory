#include <shm.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>

static bool validParams(Shared_Memory_t *shm);

bool Shared_Memory_Init(Shared_Memory_t *shm)
{
    int status = true;

    if(validParams(shm) == false)
        status = false;

    else if((shm->id = shmget(shm->projId, shm->size, shm->flags | IPC_CREAT)) == -1)
        status = false;

    else if((shm->shm = shmat(shm->id, (void *)0, 0)) == NULL)
        status = false;

    return status;
}

bool Shared_Memory_Detach(Shared_Memory_t *shm)
{
    bool status = true;

    if (!shm)
        status = false;
    else
        shmdt(shm->shm);

    return status;
}

bool Shared_Memory_Destroy(Shared_Memory_t *shm)
{
    bool status = true;

    if (!shm)
        status = false;
    else
        shmctl(shm->id, IPC_RMID, 0);

    return status;
}

static bool validParams(Shared_Memory_t *shm)
{
    bool status = true;
    if (!shm)
        status = false;

    else if(strlen(shm->path) <= 0 || strlen(shm->path) > MEMORY_PATH_LEN)
        status = false;

    else if(shm->size <= 0 || shm->size > MEMORY_PATH_LEN)
        status = false;

    return status;
}