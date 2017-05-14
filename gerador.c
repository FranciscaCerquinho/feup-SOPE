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

#define NUMBER_THREADS 2

int nrOfRequests;
int maxUse;

struct timespec ts;


struct thread_data{
	int numberOfRequests;
	int maxUse;
};

//ver passagem de parametros das threads
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
		}else{
			g = 'F';
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
	printf("%d\n",generatedRequest->nrOfRejects);
	dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - REJEITADO\n",getpid(),pthread_self(),generatedRequest->serial_number,generatedRequest->gender, generatedRequest->timeReq);
	if(generatedRequest->nrOfRejects==3){
		dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - DESCARTADO\n",getpid(),pthread_self(),generatedRequest->serial_number,generatedRequest->gender, generatedRequest->timeReq);
		close(input_fifo);
		return -1;
	}else{
        write(input_fifo, generatedRequest, sizeof(*generatedRequest));
		close(input_fifo);
		dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - PEDIDO\n",getpid(),pthread_self(),generatedRequest->serial_number,generatedRequest->gender, generatedRequest->timeReq);

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


	/*threads*/
	pthread_t newsRequest;
	pthread_t rejectedRequest;

	pthread_create(&newsRequest,NULL,thr_NewsRequest,NULL);

	pthread_create(&rejectedRequest,NULL,thr_RejectedRequest,NULL);
	/**/

	pthread_exit(0);
}
