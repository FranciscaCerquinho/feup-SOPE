#include "pedido.h"

#define NUM_CHARS_FILE_NAME 100
#define NUM_MAX_CLIENTES 10000


pthread_cond_t no_vagas = PTHREAD_COND_INITIALIZER;
int vagas;
pthread_mutex_t vagas_sem;
int testar;
pthread_mutex_t testar_sem;
int servidos_f=0;
int servidos_m=0;
struct timespec ts;
struct timespec tInicial;
void* processamento_de_pedidos(void* pedido){
    struct request naSauna = *((struct request *) pedido);
    usleep(naSauna.timeReq*1000);
    pthread_mutex_lock(&testar_sem);
    timespec_get(&ts, TIME_UTC);
    dprintf(testar,"%8.2f - %d - %ld - %4d: %c - %4d - SERVIDO\n",(ts.tv_sec - tInicial.tv_sec)*1000+(ts.tv_nsec - tInicial.tv_nsec)*0.000001,getpid(),pthread_self(),naSauna.serial_number,naSauna.gender, naSauna.timeReq);
    if(naSauna.gender=='F'){
        servidos_f++;
    }else{
        servidos_m++;
    }
    pthread_mutex_unlock(&testar_sem);
    pthread_mutex_lock(&vagas_sem);
    vagas++;
    pthread_cond_broadcast(&no_vagas);
    pthread_mutex_unlock(&vagas_sem);
    return NULL;
}

int main(int argc, char** argv){
    //Validação de variaveis
   timespec_get(&tInicial, TIME_UTC);
    if(argc != 2){
        printf("Usage: sauna <n. lugares>\n");
        return 1;
    }
    //Inicialização de mutexes e variaveis de condição
    pthread_mutex_init(&vagas_sem,NULL);
    pthread_mutex_init(&testar_sem,NULL);
    pthread_cond_init(&no_vagas,NULL);
    //Inicialização do numero de lugares
    int nLugares = atoi(argv[1]);
    pthread_mutex_lock(&vagas_sem);
    vagas=nLugares;
    pthread_mutex_unlock(&vagas_sem);

    //Criação e abretura de fifos
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

    //Criação e abretura de ficheiro de registo
    char nomeFile[NUM_CHARS_FILE_NAME];
    sprintf(nomeFile, "%s%d","/tmp/bal.", getpid());
    pthread_mutex_lock(&testar_sem);
    if((testar = open(nomeFile,O_WRONLY | O_CREAT | O_EXCL,0666)) == -1){
        perror("sauna");
        pthread_mutex_unlock(&testar_sem);
        printf("Erro ao abrir FILE - %s\n",nomeFile);
        return 1;
    }
    pthread_mutex_unlock(&testar_sem);

    //Processamento dos Pedidos
    char genero='G';
    struct request recebido[NUM_MAX_CLIENTES];
    pthread_t tid[NUM_MAX_CLIENTES];
    int i=0;
    //Variaveis para as estatisticas
    int recebidos_f=0;
    int recebidos_m=0;
    int rejeitados_f=0;
    int rejeitados_m=0;

    while(i<NUM_MAX_CLIENTES){
        //Recebe pedido
        if(read(fifo_entrada,&(recebido[i]),sizeof(recebido[0]))==0)break;
        pthread_mutex_lock(&testar_sem);
        timespec_get(&ts, TIME_UTC);
        dprintf(testar,"%8.2f - %d - %ld - %4d: %c - %4d - RECEBIDO\n",(ts.tv_sec - tInicial.tv_sec)*1000+(ts.tv_nsec - tInicial.tv_nsec)*0.000001,getpid(),pthread_self(),recebido[i].serial_number,recebido[i].gender, recebido[i].timeReq);
        if(recebido[i].gender=='F'){
            recebidos_f++;
        }else{
            recebidos_m++;
        }
        pthread_mutex_unlock(&testar_sem);

        //Se ninguem estiver na Sauna, atualiza genero
        pthread_mutex_lock(&vagas_sem);
        if(vagas==nLugares){
            pthread_mutex_unlock(&vagas_sem);
            genero=recebido[i].gender;
        }else{
            pthread_mutex_unlock(&vagas_sem);
        }

        //Se na sauna estiverem pessoas de outro genero, rejeita
        if(genero!=recebido[i].gender){
             pthread_mutex_lock(&testar_sem);
             timespec_get(&ts, TIME_UTC);
             dprintf(testar,"%8.2f - %d - %ld - %4d: %c - %4d - REJEITADO\n",(ts.tv_sec - tInicial.tv_sec)*1000+(ts.tv_nsec - tInicial.tv_nsec)*0.000001,getpid(),pthread_self(),recebido[i].serial_number,recebido[i].gender, recebido[i].timeReq);
             if(recebido[i].gender=='F'){
                rejeitados_f++;
             }else{
                rejeitados_m++;
             }
             pthread_mutex_unlock(&testar_sem);
             write(fifo_rejeitados,&(recebido[i]),sizeof(recebido[0]));
        }else
        {
            //Se existem vagas, entra na sauna
            while(1){
                pthread_mutex_lock(&vagas_sem);
                if(vagas>0){
                    vagas--;
                    pthread_mutex_unlock(&vagas_sem);
                    pthread_create(&(tid[i]), NULL, &processamento_de_pedidos, &(recebido[i]));
                    break;
                }else{
                    //Espera que existam vagas
                    pthread_cond_wait(&no_vagas,&vagas_sem);
                    pthread_mutex_unlock(&vagas_sem);
                }
            }
            i++;
        }
    }
    //Espera pela conclusão das threads
    int num_tids=i;
    for(i=0;i<num_tids;i++){
        pthread_join(tid[i],NULL);
    }

    //Imprime estatisticas
    pthread_mutex_lock(&testar_sem);
    dprintf(testar,"N. recebidos:\n%d, %d, %d\nN. rejeitados:\n%d, %d, %d\nN. servidos:\n%d, %d, %d\n"
            ,(recebidos_f+recebidos_m),recebidos_f,recebidos_m
            ,(rejeitados_f+rejeitados_m),rejeitados_f,rejeitados_m
            ,(servidos_f+servidos_m),servidos_f,servidos_m);
    pthread_mutex_unlock(&testar_sem);

    //Fechar as variaveis
    close(fifo_rejeitados);
    pthread_mutex_lock(&testar_sem);
    close(testar);
    pthread_mutex_unlock(&testar_sem);
    pthread_mutex_destroy(&testar_sem);
    pthread_mutex_destroy(&vagas_sem);
    pthread_cond_destroy(&no_vagas);
    close(fifo_entrada);
    unlink("/tmp/entrada");
    return 0;
}
