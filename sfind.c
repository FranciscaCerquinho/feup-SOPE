#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
//toupper
#include <ctype.h>
#include <xlocale.h>

#define WRITE 1
#define READ 0
#define MAX_LENGTH 1000
#define MAX_DIR 100


int pid[MAX_DIR];
int have_to_print = 0;
int have_to_delete = 0;
int have_to_exec = 0;

static unsigned long parse_ulong(char *str, int base)
{
	char *endptr;
	unsigned long val;

	/* Convert string to unsigned long */
	val = strtoul(str, &endptr, base);

	/* Check for conversion errors */
	if ((errno == ERANGE && val == ULONG_MAX) || (errno != 0 && val == 0)) {
		perror("strtoul");
		return ULONG_MAX;
	}

	if (endptr == str) {
		printf("timer: parse_ulong: no digits were found in %s\n", str);
		return ULONG_MAX;
	}

	/* Successful conversion */
	return val;
}

int isthat(char * name, char ** argv){
       if(have_to_print == 1){
	       char actualpath[MAX_LENGTH];
     	   printf("%s\n",realpath(name, actualpath));
       }else if(have_to_delete == 1){
         remove(name);
       }else if(have_to_exec == 1){
            printf("AQUI");
            char cmd[MAX_LENGTH];            printf("AQUI");
            int i=5;
            strcpy(cmd,argv[i]);            printf("AQUI");
            i++;
            printf("AQUI");
            while(strcmp(argv[i],";")){
                printf("H");
                strcat(cmd, argv[i]);i++;
            }
            printf("ENTERE\n");
            printf("|");
            printf("%s\n",cmd);
            printf("|");
       }
 return 0;
}

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
        }
        exit(0);
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



	if(!strcmp(argv[4], "-print"))
		have_to_print=1;
	else
	    have_to_print=0;
	if(!strcmp(argv[4], "-delete"))
		have_to_delete=1;
    else
        have_to_delete=0;
    if(!strcmp(argv[4], "-exec"))
        have_to_exec=1;
    else
        have_to_exec=0;


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
          if((strcmp(dentry->d_name, ".") && strcmp(dentry->d_name, ".."))){
             if((pid[i]=fork()) == 0){
                 setpgid(getpid(), getpid());
                 argv[1] = dentry->d_name;
                 execvp(argv[0], argv);
                 printf("ERRO NO EXEC\n");
                 exit(3);
             }
             else{
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


 while((dentry=readdir(dir))!=NULL){ // VE SE E ESTE E CORRE O COMMANDO
        stat(dentry->d_name,&estado);
        if(strcmp(dentry->d_name, ".") && strcmp(dentry->d_name, "..")){
        if(!strcmp("-name", argv[2])){
            if(!strcmp(dentry->d_name, argv[3]))
                isthat(dentry->d_name,argv);
        }
        else if(!strcmp("-type",argv[2])){
            if(!strcmp(argv[3],"d")){
                if(S_ISDIR(estado.st_mode))
                    isthat(dentry->d_name,argv);
            }
            else if(!strcmp(argv[3],"f")){
                if (S_ISREG(estado.st_mode))
                    isthat(dentry->d_name,argv);

            }
            else if(!strcmp(argv[3],"l")){
                if (S_ISLNK(estado.st_mode))
                    isthat(dentry->d_name,argv);
            }
        }
        else {if(!strcmp("-perm",argv[2])){
                 int perm = parse_ulong(argv[3], 8);
                 if(perm==(estado.st_mode%512))
                    isthat(dentry->d_name,argv);
             }
        }
 }}


 while(wait(NULL)!=-1){//TEM FILHOS
  ;
 }

 exit(0);
}

