#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <pthread.h>

#define NUMBER_THREADS 2

int nrOfRequests;
int maxUse;

struct timespec ts;

/*
 * struct request
 */
struct request{
	int serial_number;
	char gender;
	int timeReq;
	int nrOfRejects;
};

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

	input_fifo = open("/tmp/entrada", O_WRONLY);

	struct request generatedRequest;
	struct thread_data *threadData;

	threadData = (struct thread_data*)thread_arg;
	int nmbOfRequests = threadData->numberOfRequests;
	int maxUsage = threadData->maxUse;

	//assistant of rand
	srand(time(NULL));

	int cont = 1;
	int nSerie = 1;

	while (cont <= nmbOfRequests) {
		char g;
		int t;

		int auxG = rand() & 1;

		if(auxG == 0){
			g = 'M';
		}else{
			g = 'F';
		}

		t = rand() % maxUsage + 1;

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
//void processRejectedRequest(struct request *generatedRequest){
int processRejectedRequest(struct request *generatedRequest){
	//Increment before verifying
	generatedRequest->nrOfRejects++;
	dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - REJEITADO\n",getpid(),pthread_self(),generatedRequest->serial_number,generatedRequest->gender, generatedRequest->timeReq);
	if(generatedRequest->nrOfRejects==3){
		dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - DESCARTADO\n",getpid(),pthread_self(),generatedRequest->serial_number,generatedRequest->gender, generatedRequest->timeReq);
		return -1;
	}else{
		int input_fifo;

		input_fifo = open("/tmp/entrada", O_WRONLY);

		write(input_fifo, &generatedRequest, sizeof(generatedRequest));
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
		printf("WHY...\n");
		return 1;
	}

	int processAnswer = 0;

	while(true){
		read(fdRej,&generatedRequest,sizeof(generatedRequest));
		processAnswer = processRejectedRequest(&generatedRequest);
		//condição para parar de ler os rejeitados?

	}

	close(fdRej);
	unlink("/tmp/rejeitados");
}

int main(int argc, char const *argv[]) {

	//Data processing
	if(argc != 4){
		printf("Usage: geardor <nr. of requests> <max. use> <unit of time>\n");
		return 1;
	}

	if(atoi(argv[1]) >= 1){
		nrOfRequests = atoi(argv[1]);
	}else{
		printf("<nr. of requests> invalid\n");
		return 1;
	}

	if(atoi(argv[2]) >= 1){
		maxUse = atoi(argv[2]);
	}else{
		printf("<max. use> invalid\n");
		return 1;
	}

	char unitOfTime = argv[3][0];

	//Rejected fifo
	int fdRej;
	mkfifo("/tmp/rejeitados",0660);

	thread_data threadData;
	threadData.numberOfRequests = nrOfRequests;
	threadData.maxUse = maxUse;

	/*threads*/
	pthread_t newsRequest;
	pthread_t rejectedRequest;

	pthread_create(&newsRequest,NULL,thr_NewsRequest,(void*)&threadData);
	pthread_create(&rejectedRequest,NULL,thr_RejectedRequest,&threadData);
	/**/

	return 0;
}
