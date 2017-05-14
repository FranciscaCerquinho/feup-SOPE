#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <pthread.h>
#include "pedido.h"

#define NUM_CHARS_FILE_NAME 100
#define NUMBER_THREADS 2

int nrOfRequests;
int maxUse;
pthread_mutex_t testar_sem;
int testar;
struct timespec ts;
int generated_m = 0;
int generated_f = 0;
int rejected_m  = 0;
int rejected_f = 0;
int discarded_m = 0;
int discarded_f = 0;

struct thread_data{
	int numberOfRequests;
	int maxUse;
};

/**
 * Thread that performs the random generation of orders
 */
void *thr_NewsRequest(void *thread_arg){
	//input fifo
	int input_fifo;

	do{
		input_fifo=open("/tmp/entrada", O_WRONLY);
	}while(input_fifo==-1);

	struct request generatedRequest;


	//assistant of rand
	srand(time(NULL));

	int cont = 1;
	int nSerie = 1;

	while (cont <= nrOfRequests) {
		char g;
		int t;

		int auxG = rand() & 1;

		if(auxG == 0){
			g = 'M';
			generated_m++;
		}else{
			g = 'F';
			generated_f++;
		}

		t = rand() % maxUse + 1;

		generatedRequest.serial_number = nSerie;
		generatedRequest.gender = g;
		generatedRequest.timeReq = t;
		generatedRequest.nrOfRejects = 0;

		write(input_fifo, &generatedRequest, sizeof(generatedRequest));

		nSerie++;
		cont++;
	}

	close(input_fifo);
}

/**
 * Function used to process rejected orders
 */

int processRejectedRequest(struct request *generatedRequest){
	int input_fifo;
	do{
		input_fifo=open("/tmp/entrada", O_WRONLY);
	}while(input_fifo==-1);
	//Increment before verifying
	generatedRequest->nrOfRejects= generatedRequest->nrOfRejects+1;

	timespec_get(&ts, TIME_UTC);
	dprintf(testar,"%ld  - %d - %d - %d: %c - %d - REJEITADO\n",ts.tv_nsec,getpid(),pthread_self(),generatedRequest->serial_number,generatedRequest->gender, generatedRequest->timeReq);
	if(generatedRequest->gender== 'M')
		rejected_m++;
	else
		rejected_f++;

	if(generatedRequest->nrOfRejects==3){
		timespec_get(&ts, TIME_UTC);
		dprintf(testar,"%ld - %d - %d - %d: %c - %d - DESCARTADO\n",ts.tv_nsec,getpid(),pthread_self(),generatedRequest->serial_number,generatedRequest->gender, generatedRequest->timeReq);
		if(generatedRequest->gender=='M')
			discarded_m++;
		else
			discarded_f++;
		close(input_fifo);
		return -1;
	}
	else{
		write(input_fifo, generatedRequest, sizeof(*generatedRequest));
		close(input_fifo);
		timespec_get(&ts, TIME_UTC);
		dprintf(testar,"%ld - %d - %d - %d: %c - %d - PEDIDO\n",ts.tv_nsec,getpid(),pthread_self(),generatedRequest->serial_number,generatedRequest->gender, generatedRequest->timeReq);
		return 0;
	}
}

/**
 * Thread that checks the rejected requests and places them in the queue of requests
 */
void *thr_RejectedRequest(void *arg){
	int fdRej;

	fdRej=open("/tmp/rejeitados",O_RDONLY);

	if(fdRej == -1){
		return 1;
	}

	int processAnswer = 0;

	struct request generatedRequest;

	while(read(fdRej,&generatedRequest,sizeof(generatedRequest)) != 0){
		//read(fdRej,&generatedRequest,sizeof(generatedRequest));
		processAnswer = processRejectedRequest(&generatedRequest);
		//condição para parar de ler os rejeitados?
	}

	close(fdRej);
	unlink("/tmp/rejeitados");

	//Imprime estatisticas
	pthread_mutex_lock(&testar_sem);
	dprintf(STDOUT_FILENO,"N. gerados:\n%d, %d, %d\nN. rejeitados:\n%d, %d, %d\nN. descartados:\n%d, %d, %d\n"
			,(generated_f+generated_m),generated_f,generated_m
			,(rejected_f+rejected_m),rejected_f,rejected_m
			,(discarded_f+discarded_m),discarded_f,discarded_m);
	pthread_mutex_unlock(&testar_sem);
}

int main(int argc, char const *argv[]) {

	//Data processing
	if(argc != 3){
		return 1;
	}

	if(atoi(argv[1]) >= 1){
		nrOfRequests = atoi(argv[1]);
	}else{
		return 1;
	}

	if(atoi(argv[2]) >= 1){
		maxUse = atoi(argv[2]);
	}else{
		return 1;
	}

	//Rejected fifo
	int fdRej;
	mkfifo("/tmp/rejeitados",0660);

	//Criação e abretura de ficheiro de registo
	char nomeFile[NUM_CHARS_FILE_NAME];
	sprintf(nomeFile, "%s%d","/tmp/ger.", getpid());
	pthread_mutex_lock(&testar_sem);
	if((testar = open(nomeFile,O_WRONLY | O_CREAT | O_EXCL,0666)) == -1){
		perror("gerador");
		pthread_mutex_unlock(&testar_sem);
		printf("Erro ao abrir FILE - %s\n",nomeFile);
		return 1;
	}
	pthread_mutex_unlock(&testar_sem);

	/*threads*/
	pthread_t newsRequest;
	pthread_t rejectedRequest;

	pthread_create(&newsRequest,NULL,thr_NewsRequest,NULL);

	pthread_create(&rejectedRequest,NULL,thr_RejectedRequest,NULL);
	/**/



	pthread_exit(0);
}
