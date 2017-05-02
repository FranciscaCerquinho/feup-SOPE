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
  int serial_number;
  char gender;
  int timeReq;
};



int vagas;
sem_t vagas_sem;
int testar;
sem_t testar_sem;

//TODO RECEBEU PEDIDO
void* processamento_de_pedidos(void* pedido){
    struct request naSauna = *((struct request *) pedido);
    usleep(naSauna.timeReq); //TODO milisegundos
    sem_wait(&testar_sem);
    dprintf(testar,"inst - %d - %d - %d: %c - %d - SERVIDO\n",getpid(),pthread_self(),naSauna.serial_number,naSauna.gender, naSauna.timeReq);
    dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - SERVIDO\n",getpid(),pthread_self(),naSauna.serial_number,naSauna.gender, naSauna.timeReq);
    sem_post(&testar_sem);
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
        perror("sauna");
        printf("Erro ao iniciar o semafore - vagas\n");
        return 1;
    }
    if(sem_init(&testar_sem,0,1)==-1){
        perror("sauna");
        printf("Erro ao iniciar o semafore - testar\n");
        return 1;
    }
    int nLugares = atoi(argv[1]);
    sem_wait(&vagas_sem);
    vagas=nLugares;
    sem_post(&vagas_sem);

    ///TODO  un. tempo

    //TODO FIFOs
    if(mkfifo("/tmp/entrada",0660)){
        perror("sauna");
        printf("Erro ao criar FIFO - /tmp/entrada\n");
        return 1;
    }
    int fifo_entrada;
    if((fifo_entrada = open("/tmp/entrada",O_RDONLY)) == -1){
        perror("sauna");
        printf("Erro ao abrir FIFO - /tmp/entrada\n");
        return 1;
    }
    int fifo_rejeitados;
    do{
        fifo_rejeitados= open("/tmp/rejeitados",O_WRONLY);
    }while(fifo_rejeitados==-1);

    //TODO FILE
    char nomeFile[NUM_CHARS_FILE_NAME];
    sprintf(nomeFile, "%s%d","/tmp/bal.", getpid());
    sem_wait(&testar_sem);
    if((testar = open(nomeFile,O_WRONLY | O_CREAT | O_EXCL,0666)) == -1){
        perror("sauna");
        sem_post(&testar_sem);
        printf("Erro ao abrir FILE - %s\n",nomeFile);
        return 1;
    }
    sem_post(&testar_sem);
    //TODO PROCESSAMENTO
    char genero='G';
    struct request recebido[NUM_MAX_CLIENTES];
    pthread_t tid[NUM_MAX_CLIENTES];
    int i=0;
    while(i<NUM_MAX_CLIENTES){
        if(read(fifo_entrada,&(recebido[i]),sizeof(recebido[0]))==0)break;
        sem_wait(&testar_sem);
        dprintf(testar,"inst - %d - %d - %d: %c - %d - RECEBIDO\n",getpid(),pthread_self(),recebido[i].serial_number,recebido[i].gender, recebido[i].timeReq);
        dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - RECEBIDO\n",getpid(),pthread_self(),recebido[i].serial_number,recebido[i].gender, recebido[i].timeReq);
        sem_post(&testar_sem);
        //TODO Test gender

        sem_wait(&vagas_sem);
        if(vagas>0){
            vagas--;
            sem_post(&vagas_sem);
            pthread_create(&(tid[i]), NULL, &processamento_de_pedidos, &(recebido[i]));
        }else{
            sem_post(&vagas_sem);
            dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - ERRO\n",getpid(),pthread_self(),recebido[i].serial_number,recebido[i].gender, recebido[i].timeReq);
            //TODO Same_Gnder NO VAGAS
        }
        i++;
    }
    //TODO WAIT THEREADS
    int num_tids=i;
    for(i=0;i<num_tids;i++){
        pthread_join(tid[i],NULL);
    }
    //TODO STATISTICAS

    //TODO ACABAR variaveis
    close(fifo_rejeitados);
    sem_wait(&testar_sem);
    close(testar);
    sem_post(&testar_sem);
    sem_destroy(&vagas_sem);
    close(fifo_entrada);
    unlink("/tmp/entrada");
    return 0;
}
