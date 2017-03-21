#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>

int main(int argc,char** argv){//nome dir -procurar porcurar -fazer
 if(argc<5){
  printf("Too few arguments...\n");
  exit(1);
 }

 DIR *dir;
 struct dirent *dentry; 
 struct stat estado;

 if((dir=opendir(argv[1]))==NULL){
  printf("Nao consigo abrir o diretório %s",argv[1]);
  exit(2);
 }
 chdir(argv[1]);
 while((dentry=readdir(dir))!=NULL){
  stat(dentry->d_name,&estado);
  printf("%s\n",);///
 }
}
