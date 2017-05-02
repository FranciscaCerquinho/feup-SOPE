#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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
  printf("%d %d %c\n", nPedidos,maxUtilizacao,unTempo);
  //
  
  //entrada
  int fdEnt;
  char str[9999];

  //
  printf("AQUI\n");
  //

  fdEnt=open("/tmp/entrada",O_WRONLY);

  //
  printf("AQUI\n");
  //

  //rejeitados
  int fdRej;

  mkfifo("/tmp/rejeitados",0660);

  //
  printf("AQUI\n");
  //

  fdRej=open("/tmp/rejeitados",O_RDONLY);

  //
  printf("AQUI\n");
  //

  while(readline(fdRej,str)){
    printf("%s",str);
  }

  close(fdRej);
  close(fdEnt);

  return 0;
}
