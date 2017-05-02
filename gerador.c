#include <stdio.h>
#include <unistd.h>
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
  //entrada
  int fdEnt;
  char str[9999];

  fdEnt=open("/tmp/entrada",O_WRONLY);

  //rejeitados
  int fdRej;

  mkfifo("/tmp/rejeitados",0660);
  fdRej=open("/tmp/rejeitados",O_RDONLY);

  while(readline(fdRej,str)){
    printf("%s",str);
  }

  close(fdRej);
  close(fdEnt);

  return 0;
}
