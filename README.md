# _Shared Memory System V_

## Tópicos
* [Introdução](#introdução)
* [Shared Memory System V](#shared-memory-system-v-1)
* [Systemcalls](#systemcalls)
* [ipcs](#ipcs)
* [Implementação](#implementação)
* [shm.h](#shmh)
* [shm.c](#shmc)
* [launch_processes.c](#launch_processesc)
* [button_interface.h](#button_interfaceh)
* [button_interface.c](#button_interfacec)
* [led_interface.h](#led_interfaceh)
* [led_interface.c](#led_interfacec)
* [Compilando, Executando e Matando os processos](#compilando-executando-e-matando-os-processos)
* [Compilando](#compilando)
* [Clonando o projeto](#clonando-o-projeto)
* [Selecionando o modo](#selecionando-o-modo)
* [Modo PC](#modo-pc)
* [Modo RASPBERRY](#modo-raspberry)
* [Executando](#executando)
* [Interagindo com o exemplo](#interagindo-com-o-exemplo)
* [MODO PC](#modo-pc-1)
* [MODO RASPBERRY](#modo-raspberry-1)
* [ipcs funcionamento](#ipcs-funcionamento)
* [Matando os processos](#matando-os-processos)
* [Conclusão](#conclusão)
* [Referência](#referência)

## Introdução
Em programação Multiprocesso, cada processo(programa em execução) possui sua memória isolada dos demais processos, mas há situações que todos esses processos possuem a mesma base de dados, existem aplicações que não é interessante cada processo possuir uma cópia desses dados, pois o dado está em constante mudança sendo ele um input para todos os processos. Ao invés de enviar o mesmo dado para todos os processos interessados, sendo esses processos residentes no mesmo contexto(computador), existe um recurso chamado de Shared Memory que compartilha um região de memória entre todos os processos. Neste artigo será abordado o uso do IPC Shared Memory System V.

## Shared Memory System V
Shared Memory é um bloco de memória alocado pelo processo criador, esse bloco de memória fica acessível por meio de um identificador. Outros processos que quiserem usar essa memória deve se conectar a ela. Todo dado alterado nessa região fica disponível para todos os processos anexados.

## Systemcalls

Cria a Shared Memory apartir da chave 
```c
#include <sys/ipc.h>
#include <sys/shm.h>

int shmget(key_t key, size_t size, int shmflg);
```

Permite conectar um processo a uma Shared Memory
```c
#include <sys/types.h>
#include <sys/shm.h>

void *shmat(int shmid, const void *shmaddr, int shmflg);
```

Permite desconectar um processo de uma Shared Memory
```c
#include <sys/types.h>
#include <sys/shm.h>

int shmdt(const void *shmaddr);

```

Permite controlar os atributos de uma Shared Memory, para remover basta usar o comando IPC_RMID
```c
#include <sys/ipc.h>
#include <sys/shm.h>

int shmctl(int shmid, int cmd, struct shmid_ds *buf);
```

## ipcs
A ferramenta ipcs é um utilitário para poder verificar o estado dos IPC's sendo eles: Queues, Semaphores e Shared Memory, o seu funcionamento será demonstrado mais a seguir. Para mais informações execute:
```bash
$ man ipcs
```

## Implementação
Para facilitar a implementação a API da Shared Memory foi abstraída para facilitar o uso.

### shm.h

Aqui é definido a estrutura para guardar o contexto referente a Shared Memory
```c
#define MEMORY_PATH_LEN     1024

typedef struct
{
    char path[MEMORY_PATH_LEN]; 
    int key;                    
    int projId;                 
    int size;                   
    int flags;                  
    void *shm;                  
    int id;                     
} Shared_Memory_t;

```

Para facilitar o uso da Shared Memory foi criada uma abstração onde o Shared_Memory_Init cria e conecta a uma Shared Memory, Shared_Memory_Detach desconecta o processo da Shared Memory e Shared_Memory_Destroy remove a Shared Memory
```c
bool Shared_Memory_Init(Shared_Memory_t *shm);
bool Shared_Memory_Detach(Shared_Memory_t *shm);
bool Shared_Memory_Destroy(Shared_Memory_t *shm);
```

### shm.c 

Aqui a Shared Memory é criada e já conecta o processo
```c
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
```

Nessa função o processo é desconectado da Shared Memory
```c
bool Shared_Memory_Detach(Shared_Memory_t *shm)
{
    bool status = true;

    if (!shm)
        status = false;
    else
        shmdt(shm->shm);

    return status;
}
```

Esta função é responsável por remover a Shared Memory do sistema
```c
bool Shared_Memory_Destroy(Shared_Memory_t *shm)
{
    bool status = true;

    if (!shm)
        status = false;
    else
        shmctl(shm->id, IPC_RMID, 0);

    return status;
}

```

Para demonstrar o uso desse IPC, será utilizado o modelo Produtor/Consumidor, onde o processo Produtor(_button_process_) vai escrever seu estado em uma mensagem, e inserir na queue, e o Consumidor(_led_process_) vai ler a mensagem da queue e aplicar ao seu estado interno. A aplicação é composta por três executáveis sendo eles:

* _launch_processes_ - é responsável por lançar os processos _button_process_ e _led_process_ através da combinação _fork_ e _exec_
* _button_interface_ - é responsável por ler o GPIO em modo de leitura da Raspberry Pi e escrever o estado em uma mensagem e inserir na queue
* _led_interface_ - é responsável por ler a mensagem da queue e aplicar em um GPIO configurado como saída


### *launch_processes.c*

No _main_ criamos duas variáveis para armazenar o PID do *button_process* e do *led_process*, e mais duas variáveis para armazenar o resultado caso o _exec_ venha a falhar.
```c
int pid_button, pid_led;
int button_status, led_status;
```

Em seguida criamos um processo clone, se processo clone for igual a 0, criamos um _array_ de *strings* com o nome do programa que será usado pelo _exec_, em caso o _exec_ retorne, o estado do retorno é capturado e será impresso no *stdout* e aborta a aplicação. Se o _exec_ for executado com sucesso o programa *button_process* será carregado. 
```c
pid_button = fork();

if(pid_button == 0)
{
    //start button process
    char *args[] = {"./button_process", NULL};
    button_status = execvp(args[0], args);
    printf("Error to start button process, status = %d\n", button_status);
    abort();
}   
```

O mesmo procedimento é repetido novamente, porém com a intenção de carregar o *led_process*.

```c
pid_led = fork();

if(pid_led == 0)
{
    //Start led process
    char *args[] = {"./led_process", NULL};
    led_status = execvp(args[0], args);
    printf("Error to start led process, status = %d\n", led_status);
    abort();
}
```

## *button_interface.h*
Para usar a interface do botão precisa implementar essas duas callbacks para permitir o seu uso
```c
typedef struct 
{
    bool (*Init)(void *object);
    bool (*Read)(void *object);
    
} Button_Interface;
```

A assinatura do uso da interface corresponde ao contexto do botão, que depende do modo selecionado, o contexo da Shared Memory, e a interface do botão devidamente preenchida.
```c
bool Button_Run(void *object, Shared_Memory_t *shm, Button_Interface *button);
```

## *button_interface.c*
A implementação da interface baseia-se em inicializar o botão, inicializar a Shared Memory, e no loop realiza a escrita na Shared Memory mediante o pressionamento do botão.
```c
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
```

## *led_interface.h*
Para realizar o uso da interface de LED é necessário preencher os callbacks que serão utilizados pela implementação da interface, sendo a inicialização e a função que altera o estado do LED.
```c
typedef struct 
{
    bool (*Init)(void *object);
    bool (*Set)(void *object, uint8_t state);
} LED_Interface;
```

A assinatura do uso da interface corresponde ao contexto do LED, que depende do modo selecionado, o contexo da Shared Memory, e a interface do LED devidamente preenchida.
```c
bool LED_Run(void *object, Shared_Memory_t *shm, LED_Interface *led);
```

## *led_interface.c*
A implementação da interface baseia-se em inicializar o LED, inicializar a Shared Memory, e no loop realiza a leitura do conteúdo da Shared Memory
```c
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
```

## Compilando, Executando e Matando os processos
Para compilar e testar o projeto é necessário instalar a biblioteca de [hardware](https://github.com/NakedSolidSnake/Raspberry_lib_hardware) necessária para resolver as dependências de configuração de GPIO da Raspberry Pi.

## Compilando
Para facilitar a execução do exemplo, o exemplo proposto foi criado baseado em uma interface, onde é possível selecionar se usará o hardware da Raspberry Pi 3, ou se a interação com o exemplo vai ser através de input feito por FIFO e o output visualizado através de LOG.

### Clonando o projeto
Pra obter uma cópia do projeto execute os comandos a seguir:

```bash
$ git clone https://github.com/NakedSolidSnake/Raspberry_IPC_SharedMemory_SystemV
$ cd Raspberry_IPC_SharedMemory_SystemV
$ mkdir build && cd build
```

### Selecionando o modo
Para selecionar o modo devemos passar para o cmake uma variável de ambiente chamada de ARCH, e pode-se passar os seguintes valores, PC ou RASPBERRY, para o caso de PC o exemplo terá sua interface preenchida com os sources presentes na pasta src/platform/pc, que permite a interação com o exemplo através de FIFO e LOG, caso seja RASPBERRY usará os GPIO's descritos no [artigo](https://github.com/NakedSolidSnake/Raspberry_lib_hardware#testando-a-instala%C3%A7%C3%A3o-e-as-conex%C3%B5es-de-hardware).

#### Modo PC
```bash
$ cmake -DARCH=PC ..
$ make
```

#### Modo RASPBERRY
```bash
$ cmake -DARCH=RASPBERRY ..
$ make
```

## Executando
Para executar a aplicação execute o processo _*launch_processes*_ para lançar os processos *button_process* e *led_process* que foram determinados de acordo com o modo selecionado.

```bash
$ cd bin
$ ./launch_processes
```

Uma vez executado podemos verificar se os processos estão rodando atráves do comando 
```bash
$ ps -ef | grep _process
```

O output 
```bash
cssouza  15938  2417  0 06:33 pts/11   00:00:00 ./button_process
cssouza  15939  2417  2 06:33 pts/11   00:00:00 ./led_process
```
## Interagindo com o exemplo
Dependendo do modo de compilação selecionado a interação com o exemplo acontece de forma diferente

### MODO PC
Para o modo PC, precisamos abrir um terminal e monitorar os LOG's
```bash
$ sudo tail -f /var/log/syslog | grep LED
```

Dessa forma o terminal irá apresentar somente os LOG's referente ao exemplo.

Para simular o botão, o processo em modo PC cria uma FIFO para permitir enviar comandos para a aplicação, dessa forma todas as vezes que for enviado o número 0 irá logar no terminal onde foi configurado para o monitoramento, segue o exemplo
```bash
$ echo '0' > /tmp/shm_file
```

Output dos LOG's quando enviado o comando algumas vezez
```bash
Jul  2 06:34:44 dell-cssouza LED SHM[15939]: LED Status: On
Jul  2 06:34:45 dell-cssouza LED SHM[15939]: LED Status: Off
Jul  2 06:34:46 dell-cssouza LED SHM[15939]: LED Status: On
Jul  2 06:34:46 dell-cssouza LED SHM[15939]: LED Status: Off
Jul  2 06:34:46 dell-cssouza LED SHM[15939]: LED Status: On
Jul  2 06:34:47 dell-cssouza LED SHM[15939]: LED Status: Off
```

### MODO RASPBERRY
Para o modo RASPBERRY a cada vez que o botão for pressionado irá alternar o estado do LED.

## ipcs funcionamento
Para inspecionar as shared memories presentes é necessário passar o argumento -m que representa queue, o comando fica dessa forma:
```bash
$ ipcs -m
```
O Output gerado na máquina onde o exemplo foi executado, é possível notar que existem outros processos usando Shared Memory.
```bash
------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x00000000 688130     cssouza    600        4032432    2          dest         
0x00000000 65539      cssouza    600        188416     2          dest         
0x00000000 65540      cssouza    600        188416     2          dest         
0x00000000 229381     cssouza    600        524288     2          dest         
0x00000000 98311      cssouza    600        57200      2          dest         
0x00000000 720904     cssouza    600        380928     2          dest
...
```

Sabendo o PID é possível verificar qual processo escreveu e qual processo realizou a última alteração no conteúdo
```bash
$ ipcs -m -p
```

O output gerado.
```bash
------ Shared Memory Creator/Last-op PIDs --------
shmid      owner      cpid       lpid      
688130     cssouza    24503      2437      
65539      cssouza    4125       14845  
```
Neste caso a saída possui vários resultados. Para conseguir informações sobre o processo em questão é necessário filtrar
```bash
$ ipcs -m -p | grep 15939
```
O output gerado corresponde ao PID do botão, que alterou o conteúdo.
```bash
720983     cssouza    15939      15938 
```

## Matando os processos
Para matar os processos criados execute o script kill_process.sh
```bash
$ cd bin
$ ./kill_process.sh
```

## Conclusão
Shared Memory é extremamente útil quando se precisa de performance no acesso aos dados, sendo um ótimo recurso para o compartilhamento de dados. Este recurso deve ser usado com muita cautela, se houver mais de um processo alterando os dados é necessário um mecanismo de sincronização([semphores](https://github.com/NakedSolidSnake/Raspberry_IPC_Semaphore_SystemV)) para que não haja concorrência no acesso às informações. Este é o último IPC do padrão System V. No próximo artigo será abordado o padrão POSIX.

## Referência
* [Link do projeto completo](https://github.com/NakedSolidSnake/Raspberry_IPC_SharedMemory_SystemV)
* [Mark Mitchell, Jeffrey Oldham, and Alex Samuel - Advanced Linux Programming](https://www.amazon.com.br/Advanced-Linux-Programming-CodeSourcery-LLC/dp/0735710430)
* [fork, exec e daemon](https://github.com/NakedSolidSnake/Raspberry_fork_exec_daemon)
* [biblioteca hardware](https://github.com/NakedSolidSnake/Raspberry_lib_hardware)
