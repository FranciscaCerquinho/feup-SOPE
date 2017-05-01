#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>

#define NUM_CHARS_FILE_NAME 100
#define NUM_MAX_CLIENTES 100



struct request{
 char gender;
 int time;
 int serial_number;
};



int vagas;
sem_t vagas_sem;
int testar;

//TODO RECEBEU PEDIDO
void* processamento_de_pedidos(void* pedido){
    struct request naSauna = *((struct request *) pedido);
    usleep(naSauna.time); //TODO milisegundos
    dprintf(testar,"inst – %d – %d – %d: %c – %d – SERVIDO\n",getpid(),pthread_self(),naSauna.serial_number,naSauna.gender, naSauna.time);
    sem_wait(&vagas_sem);
    vagas++;
    sem_post(&vagas_sem);
    return NULL;
}

int main(int argc, char** argv){
    if(argc != 3){
        printf("Usage: sauna <n. lugares> <un. tempo>\n");
        return 1;
    }

    //TODO nLugares
    if(sem_init(&vagas_sem,0,1)==-1){
        printf("Erro ao iniciar o semafore\n");
        return 1;
    }
    int nLugares = atoi(argv[1]);
    sem_wait(&vagas_sem);
    vagas=nLugares;
    sem_post(&vagas_sem);

    ///TODO  un. tempo

    //TODO FIFOs
    if(mkfifo("/tmp/entrada",0660)){
        printf("Erro ao criar FIFO - /tmp/entrada\n");
        return 1;
    }
    int fifo_entrada = open("/tmp/entrada",O_RDONLY);//OPEN to WRITE!!
    if(fifo_entrada == -1){
        printf("Erro ao abrir FIFO - /tmp/entrada\n");
        return 1;
    }
    int fifo_rejeitados = open("/tmp/rejeitados",O_WRONLY);
    if(fifo_rejeitados == -1){
        printf("Erro ao abrir FIFO - /tmp/rejeitados\n");
        return 1;
    }

    //TODO FILE
    char nomeFile[NUM_CHARS_FILE_NAME];
    sprintf(nomeFile, "%s%d","/tmp/bal.", getpid());
    testar = open(nomeFile,O_WRONLY | O_CREAT | O_EXCL);
    if(testar == -1){
        printf("Erro ao abrir FILE - %s\n",nomeFile);
        return 1;
    }

    //TODO PROCESSAMENTO
    char genero;
    struct request recebido[NUM_MAX_CLIENTES];
    pthread_t tid[NUM_MAX_CLIENTES];
    int i=0;
    while(1){
        read(fifo_entrada,&(recebido[i]),sizeof(recebido[0]));
        i++;
        //TODO Test gender
        sem_wait(&vagas_sem);
        if(vagas>0){
            vagas--;
            sem_post(&vagas_sem);
            dprintf(testar,"inst – %d – %d – %d: %c – %d – RECEBIDO\n",getpid(),pthread_self(),recebido[i].serial_number,recebido[i].gender, recebido[i].time);
            pthread_create(&(tid[i]), NULL, &processamento_de_pedidos, &(recebido[i]));
        }else{
            //TODO Same_Gnder NO VAGAS
        }
    }
    //TODO WAIT THEREADS

    //TODO STATISTICAS

    //TODO ACABAR variaveis
    close(fifo_rejeitados);
    close(testar);
    sem_destroy(&vagas_sem);
    close(fifo_entrada);
    unlink("/tmp/entrada");
    return 0;
}
