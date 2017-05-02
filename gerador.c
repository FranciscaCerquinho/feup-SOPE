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

  int fd;
  char str[9999];

  mkfifo("/tmp/rejeitados",0660);
  fd=open("/tmp/rejeitados",O_RDONLY);

  while(readline(fd,str)){
    printf("%s",str);
  }

  close(fd);

  return 0;
}
