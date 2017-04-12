#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
//toupper
#include <ctype.h>
#include <xlocale.h>

#define WRITE 1
#define READ 0
#define MAX_LENGTH 1
#define MAX_DIR 100


 int pid[MAX_DIR];


//CTRL+C
void sigint_handler(int signo){
  char r;
  printf("\nAre you sure you want to terminate (Y/N)?");
  scanf("%c",&r);
  if(r == 'Y' || r=='y'){
   printf("ENTREI\n");
   int i=0;
   for(;i<MAX_DIR;i++){
    printf("A");
    if(kill(pid[i], SIGUSR1)==-1){
     printf("\n");
     exit(0);
    }
  } exit(0);
  }else if(r == 'N' || r=='n'){
    exit(5);
  }
}

//SIG-USER1
void sig_handler(int signo){
int i=0;
  for(;i<MAX_DIR;i++){
   if(kill(pid[i], SIGUSR1)==-1){
   printf("\n");
   exit(0);}
  } exit(0);
  exit(6);
}

int main(int argc,char** argv){//nome dir -procurar porcurar -fazer

 if(argc<5){
  printf("Too few arguments...\n");
  exit(1);
 }

 int i;
 int have_to_print = 0;
 int have_to_delete = 0;

 for(i=0 ; i < argc ; i++){
	if(strcmp(argv[i], "-print"))
		have_to_print=1;
	else if(strcmp(argv[i], "-delete"))
		have_to_delete=1;
 }

 if(signal(SIGINT,sigint_handler) < 0){
   fprintf(stderr, "Unable to install SIGNINT handler\n");
   exit(4);
 }
 if(signal(SIGUSR1,sig_handler) < 0){
    fprintf(stderr, "Unable to install SIG handler\n");
    exit(4);
  }

 DIR *dir;
 struct dirent *dentry;
 struct stat estado;

 if((dir=opendir(argv[1]))==NULL){
  printf("Nao consigo abrir o diretório %s",argv[1]);
  exit(2);
 }
 chdir(argv[1]);
i=0;
 while((dentry = readdir(dir))!= NULL){ //ABRE OS DIRETORIOS E FAZ EXEC
  stat(dentry->d_name, &estado);

  if(S_ISDIR(estado.st_mode)){
    //"." diretorio atual ".." diretorio acima
   if((strcmp(dentry->d_name, ".") && strcmp(dentry->d_name, ".."))){
    if((pid[i]=fork()) == 0){
     setpgid(getpid(), getpid());

     argv[1] = dentry->d_name;

     execvp(argv[0], argv);

     printf("ERRO NO EXEC\n");

     exit(3);
    }else{
    i++;
    if(i>=MAX_DIR){
    printf("DEMASIADOS DIRETIRIOS\n");
    exit(9);
    }
    }
   }
  }
 }

 rewinddir(dir);

//stat(dentry->d_type,&estado);

 while((dentry=readdir(dir))!=NULL){ // VE SE E ESTE E CORRE O COMMANDO
  stat(dentry->d_name,&estado);
 // printf("ABRIU %s\n",dentry->d_name);
  if(!strcmp(dentry->d_name, argv[3])){
	  if(have_to_print == 1)
     	   printf("%s\n",dentry->d_name);
  }/*else if(!strcmp(dentry->d_type, argv[3])){
	  if(have_to_print == 1)
     	   printf("%s",dentry->d_name);
//  if((estado.st_mode & S_IWOTH)==S_IWOTH)
}*/
}

//sleep(10);

 while(wait(NULL)!=-1){//TEM FILHOS
  ;
 }

 exit(0);
}
