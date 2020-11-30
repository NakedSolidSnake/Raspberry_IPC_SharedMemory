
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