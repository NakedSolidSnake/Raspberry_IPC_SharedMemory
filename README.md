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

### launch_processes.c
```c
/**
 * @file launch_processes.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2020-02-03
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int pid_button, pid_led;
    int button_status, led_status;

    pid_button = fork();

    if(pid_button == 0)
    {
        //start button process
        char *args[] = {"./button_process", NULL};
        button_status = execvp(args[0], args);
        printf("Error to start button process, status = %d\n", button_status);
        abort();
    }   

    pid_led = fork();

    if(pid_led == 0)
    {
        //Start led process
        char *args[] = {"./led_process", NULL};
        led_status = execvp(args[0], args);
        printf("Error to start led process, status = %d\n", led_status);
        abort();
    }

    return EXIT_SUCCESS;
}
```

### button_process.c
```c
/**
 * @file button_process.c
 * @author Cristiano Silva de Souza (cristianosstec@gmail.com)
 * @brief Realiza a escrita na shared memory através do 
 * pressionamento de um botão.
 * @version 1.0
 * @date 2020-02-03
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <button.h>
#include <shm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define _1MS    1000
#define BUFFER_SIZE     1024

static void inputHandler(void);

static Button_t button = {
        .gpio.pin = 7,
        .gpio.eMode = eModeInput,
        .ePullMode = ePullModePullUp,
        .eIntEdge = eIntEdgeFalling,
        .cb = inputHandler
    };

static SHMemory_t shm = {
    .path = "/shm_gpio",
    .projId = 65,
    .size = 1024,
    .flags = 0666
};

static char buf[BUFFER_SIZE] = {0}; /*!< Buffer para a escrita do dado */

int main(int argc, char const *argv[])
{

    if(Button_init(&button))
        return EXIT_FAILURE;

    if(SHMemory_init(&shm))
        return EXIT_FAILURE;

    while(1)
        usleep(_1MS);

    SHMemory_detach(&shm);
    SHMemory_destroy(&shm);
    
    return 0;
}

static void inputHandler(void)
{
    static int state = 0;
    if(!Button_read(&button)){
        usleep(_1MS * 40);
        while(!Button_read(&button));
        usleep(_1MS * 40);
        state ^= 0x01;

        memset(buf, 0, sizeof(buf));
        snprintf(buf, BUFFER_SIZE, "state = %d\n", state);
        strncpy(shm.shm, buf, shm.size);
    }
}

```
### led_process.c
```c
/**
 * @file led_process.c
 * @author Cristiano Silva de Souza (cristianosstec@gmail.com)
 * @brief Realiza a leitura da Shared Memory em modo
 * polling e aplica o estado ao led,
 * sendo seu estado alterado pelo processo de 
 * button_process
 * @version 1.0
 * @date 2020-02-03
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <led.h>
#include <shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

LED_t led =
    {
        .gpio.pin = 0,
        .gpio.eMode = eModeOutput
    };

static SHMemory_t shm =
    {
    .path = "/shm_gpio",
    .projId = 65,
    .size = 1024,
    .flags = 0666
    };

int main(int argc, char const *argv[])
{
    int state_cur;
    int state_old;

    if(LED_init(&led))
        return EXIT_FAILURE;

    if(SHMemory_init(&shm))
        return EXIT_FAILURE;

    while(1)
    {
        sscanf(shm.shm, "state = %d", &state_cur);
        if(state_cur !=  state_old)
        {

            state_old = state_cur;
            LED_set(&led, (eState_t)state_cur);            
        }
        usleep(1);
    }

    SHMemory_detach(&shm);    

    return EXIT_SUCCESS;
}

```


## Conclusão
