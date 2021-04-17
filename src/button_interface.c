#include <button_interface.h>
#include <shm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define _1ms    1000

static void wait_press(void *object, Button_Interface *button)
{
    while (true)
    {
        if (!button->Read(object))
        {
            usleep(_1ms * 100);
            break;
        }
        else
        {
            usleep(_1ms);
        }
    }
}

bool Button_Run(void *object, Shared_Memory_t *shm, Button_Interface *button)
{
    static int state = 0;

    if(button->Init(object) == false)
        return false;

    if(Shared_Memory_Init(shm) == false)
        return false;

    while(true)
    {
        wait_press(object, button);

        state ^= 0x01;

        snprintf(shm->shm, shm->size, "state = %d", state);
    }

    Shared_Memory_Detach(shm);
    Shared_Memory_Destroy(shm);

    return false;    
}
