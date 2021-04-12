#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h>
#include <syslog.h>
#include <led_interface.h>

bool Init(void *object);
bool Set(void *object, uint8_t state);

int main(int argc, char *argv[])
{   
    Shared_Memory_t shm = {
        .path = "/shm_gpio",
        .projId = 65,
        .size = 1024,
        .flags = 0666
    };

    LED_Interface led_interface = 
    {
        .Init = Init,
        .Set = Set
    };

    LED_Run(NULL, &shm, &led_interface);
    
    return 0;
}

bool Init(void *object)
{
    (void)object; 
    return true;
}

bool Set(void *object, uint8_t state)
{
    (void)object;    
    openlog("LED SHM", LOG_PID | LOG_CONS , LOG_USER);
    syslog(LOG_INFO, "LED Status: %s", state ? "On": "Off");
    closelog(); 
    return true;
}
