#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <pthread.h>

/*
 * struct request
 */
struct request{
  int serial_number;
  char gender;
  int timeReq;
  int nRejeitados;
};

//ver passagem de parametros das threads

void *thr_NewsRequest(void *arg){
  //fifo de entrada
  int fdEnt;

  fdEnt = open("/tmp/entrada", O_WRONLY);

  struct request geradoReq;

  //auxiliar de rand
  srand(time(NULL));

  int cont = 1;
  int nSerie = 1;

  while (cont <= nPedidos) {
    char g;
    int t;

    int auxG = rand() & 1;

    /**/
    //printf("Gender= %d\n", auxG);
    /**/

    if(auxG == 0){
      g = 'M';
    }else{
      g = 'F';
    }

    t = rand() % maxUtilizacao + 1;

    /**/
    //printf("time= %d\n", t);
    /**/

    geradoReq.serial_number = nSerie;
    geradoReq.gender = g;
    geradoReq.timeReq = t;
    geradoReq.nRejeitados = 0;

    write(fdEnt, &geradoReq, sizeof(geradoReq));
    //dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - SERVIDO\n",getpid(),pthread_self(),geradoReq.serial_number,geradoReq.gender, geradoReq.timeReq);

    nSerie++;
    cont++;
  }

  close(fdEnt);
}

void *thr_RejectedRequest(void *arg){
  int fdRej;
  //mkfifo("/tmp/rejeitados",0660);

  fdRej=open("/tmp/rejeitados",O_RDONLY);

  if(fdRej == -1){
    printf("WHY...\n");
    return 1;
  }

  read(fdRej,&geradoReq,sizeof(geradoReq));

  close(fdRej);
  unlink("/tmp/rejeitados");
}

int main(int argc, char const *argv[]) {

  //tratamento dados
  if(argc != 4){
    printf("Usage: geardor <n. Pedidos> <max. utilizacao> <un. tempo>\n");
    return 1;
  }

  int nPedidos;

  if(atoi(argv[1]) >= 1){
    nPedidos = atoi(argv[1]);
  }else{
    printf("<n. Pedidos> invalid\n");
    return 1;
  }

  int maxUtilizacao;

  if(atoi(argv[2]) >= 1){
    maxUtilizacao = atoi(argv[2]);
  }else{
    printf("<max. utilizacao> invalid\n");
    return 1;
  }

  char unTempo = argv[3][0];

  //fifo de rejeitados
  int fdRej;
  mkfifo("/tmp/rejeitados",0660);

  /*threads*/
  pthread_t newsRequest;
  pthread_t rejectedRequest;

  pthread_create(&newsRequest,NULL,thr_NewsRequest,&nPedidos);
  pthread_create(&rejectedRequest,NULL,thr_RejectedRequest,NULL);
  /**/

  /**/
  //printf("nPedidos=%d\nmaxUtilizacao=%d\nunTempo=%c\n", nPedidos,maxUtilizacao,unTempo);
  /**/

/*
  //fifo de entrada
  int fdEnt;
  char str[9999];
*/

  /**/
  //printf("AQUI 1\n");
  /**/

  //fdEnt = open("/tmp/entrada", O_WRONLY);

  /**/
  //printf("AQUI 2 \n");
  /**/

  /**/
  //printf("AQUI 3\n");
  /**/

  /*
  fdRej=open("/tmp/rejeitados",O_RDONLY);

  if(fdRej == -1){
    printf("WHY...\n");
    return 1;
  }
  */

  /**/
  //printf("AQUI\n");
  /**/

/*
  struct request geradoReq;

  //auxiliar de rand
  srand(time(NULL));

  int cont = 1;
  int nSerie = 1;

  while (cont <= nPedidos) {
    char g;
    int t;

    int auxG = rand() & 1;

    //
    //printf("Gender= %d\n", auxG);
    //

    if(auxG == 0){
      g = 'M';
    }else{
      g = 'F';
    }

    t = rand() % maxUtilizacao + 1;

    //
    //printf("time= %d\n", t);
    //

    geradoReq.serial_number = nSerie;
    geradoReq.gender = g;
    geradoReq.timeReq = t;
    geradoReq.nRejeitados = 0;

    write(fdEnt, &geradoReq, sizeof(geradoReq));
    //dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - SERVIDO\n",getpid(),pthread_self(),geradoReq.serial_number,geradoReq.gender, geradoReq.timeReq);

    nSerie++;
    cont++;
  }
  */

  /*
  //tratamento de rejeitados//
  read(fdRej,&geradoReq,sizeof(geradoReq));
  //
  */

  //close(fdRej);
  //close(fdEnt);
  //unlink("/tmp/rejeitados");

  return 0;
}
