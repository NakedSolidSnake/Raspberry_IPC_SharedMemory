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

static Button_t button7 = {
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

    if(Button_init(&button7))
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
    if(!Button_read(&button7)){
        usleep(_1MS * 40);
        while(!Button_read(&button7));
        usleep(_1MS * 40);
        state ^= 0x01;

        memset(buf, 0, sizeof(buf));
        snprintf(buf, BUFFER_SIZE, "state = %d\n", state);
        strncpy(shm.shm, buf, shm.size);
    }
}
