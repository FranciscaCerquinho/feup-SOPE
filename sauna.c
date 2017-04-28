#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#define NUM_CHARS_FILE_NAME 100
struct pedido{

};
int main(int argc, char** argv){
    if(argc != 3){
        printf("Usage: sauna <n. lugares> <un. tempo>\n");
        return 1;
    }
    int nLugares = atoi(argv[1]);
    ///TODO  un. tempo
    //TODO FIFOs
    if(mkfifo("/tmp/entrada",0660)){
        printf("Erro ao criar FIFO - /tmp/entrada\n");
        return 1;
    }
    printf("AQUI\n");
    int fifo_entrada = open("/tmp/entrada",O_RDONLY);//OPEN to WRITE!!
    printf("AQUI\n");
    if(fifo_entrada == -1){
        printf("Erro ao abrir FIFO - /tmp/entrada\n");
        return 1;
    }
    printf("AQUI\n");
    int fifo_rejeitados = open("/tmp/rejeitados",O_WRONLY);
    printf("AQUI\n");
    if(fifo_rejeitados == -1){
        printf("Erro ao abrir FIFO - /tmp/rejeitados\n");
        return 1;
    }

    //TODO FILE
    char nomeFile[NUM_CHARS_FILE_NAME];
    strcat(nomeFile,"/tmp/bal");
    strcat(nomeFile, getpid());
    int testar = open("/tmp/bal.pid",O_WRONLY | O_CREAT | O_EXCL);
    if(testar == -1){
        printf("Erro ao abrir FILE - %s\n",nomeFile);
        return 1;
    }

    //TODO PROCESSAMENTO


    //TODO WAIT THEREADS

    //TODO STATISTICAS
    return 0;
}