#include <led_interface.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define _1ms    1000

bool LED_Run(void *object, Shared_Memory_t *shm, LED_Interface *led)
{
    int state_cur;
    int state_old;

    if(led->Init(object) == false)
        return false;

    if(Shared_Memory_Init(shm) == false)
        return false;

    while(true)
    {
        sscanf(shm->shm, "state = %d", &state_cur);
        if(state_cur !=  state_old)
        {
            state_old = state_cur;
            led->Set(object, state_cur);
        }
        usleep(_1ms);
    }

    Shared_Memory_Detach(shm);  
    return false;
}

