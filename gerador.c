#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

int readline(int fd, char *str){
  int n;
  do{
    n = read(fd,str,1);
  }

  while (n>0 && *str++ != '\0');

  return (n>0);
}

struct request{
  int serial_number;
  char gender;
  int timeReq;
};



int main(int argc, char const *argv[]) {

  //tratamento dados
  if(argc != 4){
      printf("Usage: geardor <n. Pedidos> <max. utilizacao> <un. tempo>\n");
      return 1;
  }

  int nPedidos=atoi(argv[1]);
  int maxUtilizacao=atoi(argv[2]);
  char unTempo=argv[3][0];

  //
  printf("nPedidos=%d\nmaxUtilizacao=%d\nunTempo=%c\n", nPedidos,maxUtilizacao,unTempo);
  //

  //entrada
  int fdEnt;
  char str[9999];

  //
  printf("AQUI 1\n");
  //

  fdEnt=open("/tmp/entrada",O_WRONLY);

   //
    printf("AQUI 2 \n");
    //

    //rejeitados
    int fdRej;

    mkfifo("/tmp/rejeitados",0660);

    //
    printf("AQUI 3\n");
    //

    fdRej=open("/tmp/rejeitados",O_RDONLY);
    if(fdRej==-1){
      printf("WHY....,.\n");
      return 1;
    }
    //
    printf("AQUI\n");
    //



  struct request geradoReq;

  time_t t;
  srand(&t);

  int cont = 1;
  int nSerie = 1;

  while (cont<=nPedidos) {
    char g;
    int t;

    int auxG= rand() & 1;

    //
    //printf("Gender= %d\n", auxG);
    //

    if(auxG==0){
      g='M';
    }else{
      g='F';
    }

    t= rand() % maxUtilizacao + 1;

    //
    //printf("time= %d\n", t);
    //

    geradoReq.serial_number = nSerie;
    geradoReq.gender = g;
    geradoReq.timeReq = t;

    write(fdEnt,&geradoReq,sizeof(geradoReq));
    dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - SERVIDO\n",getpid(),pthread_self(),geradoReq.serial_number,geradoReq.gender, geradoReq.timeReq);
        
    nSerie++;
    cont++;
  }

  /* while(readline(fdRej,str)){
    printf("%s",str);
  }*/

  close(fdRej);
  close(fdEnt);
  unlink("/tmp/rejeitados");

  return 0;
}
