#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
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
 
 while((dentry=readdir(dir))!=NULL){ //ABRE OS DIRETORIOS E FAZ EXEC
  stat(dentry->d_name,&estado);
//  printf("ABRIO %s\n",dentry->d_name);
  if(S_ISDIR(estado.st_mode)){
   if(!strcmp(dentry->d_name,".")||!strcmp(dentry->d_name,"..")){
    ;
   }
   else{
    ////SE E DIRETORIO
    if(fork()==0){
//     printf("dentry %s\n",dentry->d_name);
     argv[1]=dentry->d_name;
     execvp(argv[0],argv);
     printf("ERRO NO EXEC\n");
     exit(3);
    }
   }
  }
 }

 while((dentry=readdir(dir))!=NULL){ // VE SE E ESTE E CORRE O COMMANDO
  stat(dentry->d_name,&estado);
  printf("ABRIO %s\n",dentry->d_name);
 }

 while(wait(NULL)!=-1){//TEM FILHOS
  ;
 }
 exit(0);
}
