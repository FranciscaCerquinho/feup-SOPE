#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <pthread.h>

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

//ver passagem de parametros das threads

void *thr_NewsRequest(void *arg){
	//input fifo
	int input_fifo;

	input_fifo = open("/tmp/entrada", O_WRONLY);

	struct request generatedRequest;

	//assistant of rand
	srand(time(NULL));

	int cont = 1;
	int nSerie = 1;

	while (cont <= nrRequest) {
		char g;
		int t;

		int auxG = rand() & 1;

		/**/
		//printf("Gender= %d\n", auxG);
		/**/

		if(auxG == 0){
			g = 'M';
		}else{
			g = 'F';
		}

		t = rand() % maxUtilizacao + 1;

		/**/
		//printf("time= %d\n", t);
		/**/

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

//Function used to process rejected orders
void processRejectedRequest(request *generatedRequest){
	//Increment before verifying
	generatedRequest->nrOfRejects++;
	dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - REJEITADO\n",getpid(),pthread_self(),generatedRequest->serial_number,generatedRequest->gender, generatedRequest->timeReq);
	if(generatedRequest->nrOfRejects==3){
		dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - DESCARTADO\n",getpid(),pthread_self(),generatedRequest->serial_number,generatedRequest->gender, generatedRequest->timeReq);
		return -1;
	}
	else
	{
		int input_fifo;

		input_fifo = open("/tmp/entrada", O_WRONLY);

		write(input_fifo, &generatedRequest, sizeof(generatedRequest));
		close(input_fifo);
		dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - PEDIDO\n",getpid(),pthread_self(),generatedRequest->serial_number,generatedRequest->gender, generatedRequest->timeReq);
	}
}

void *thr_RejectedRequest(void *arg){
	int fdRej;
	//mkfifo("/tmp/rejeitado
	s",0660);

	fdRej=open("/tmp/rejeitados",O_RDONLY);

	if(fdRej == -1){
		printf("WHY...\n");
		return 1;
	}

	while(true){
		read(fdRej,&generatedRequest,sizeof(generatedRequest));
		processRejectedRequest(&generatedRequest);
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

	int nrOfRequests;

	if(atoi(argv[1]) >= 1){
		nrOfRequests = atoi(argv[1]);
	}else{
		printf("<nr. of requests> invalid\n");
		return 1;
	}

	int maxUse;

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

	/*threads*/
	pthread_t newsRequest;
	pthread_t rejectedRequest;

	pthread_create(&newsRequest,NULL,thr_NewsRequest,&nrOfRequests);
	pthread_create(&rejectedRequest,NULL,thr_RejectedRequest,NULL);
	/**/

	/**/
	//printf("nPedidos=%d\nmaxUtilizacao=%d\nunTempo=%c\n", nPedidos,maxUtilizacao,unTempo);
	/**/

	/*
  //fifo de entrada
  int fdEnt;
  char str[9999];
	 */

	/**/
	//printf("AQUI 1\n");
	/**/

	//fdEnt = open("/tmp/entrada", O_WRONLY);

	/**/
	//printf("AQUI 2 \n");
	/**/

	/**/
	//printf("AQUI 3\n");
	/**/

	/*
  fdRej=open("/tmp/rejeitados",O_RDONLY);

  if(fdRej == -1){
    printf("WHY...\n");
    return 1;
  }
	 */

	/**/
	//printf("AQUI\n");
	/**/

	/*
  struct request geradoReq;

  //auxiliar de rand
  srand(time(NULL));

  int cont = 1;
  int nSerie = 1;

  while (cont <= nPedidos) {
    char g;
    int t;

    int auxG = rand() & 1;

    //
    //printf("Gender= %d\n", auxG);
    //

    if(auxG == 0){
      g = 'M';
    }else{
      g = 'F';
    }

    t = rand() % maxUtilizacao + 1;

    //
    //printf("time= %d\n", t);
    //

    geradoReq.serial_number = nSerie;
    geradoReq.gender = g;
    geradoReq.timeReq = t;
    geradoReq.nRejeitados = 0;

    write(fdEnt, &geradoReq, sizeof(geradoReq));
    //dprintf(STDOUT_FILENO,"inst - %d - %d - %d: %c - %d - SERVIDO\n",getpid(),pthread_self(),geradoReq.serial_number,geradoReq.gender, geradoReq.timeReq);

    nSerie++;
    cont++;
  }
	 */

	/*
  //tratamento de rejeitados//
  read(fdRej,&geradoReq,sizeof(geradoReq));
  //
	 */

	//close(fdRej);
	//close(fdEnt);
	//unlink("/tmp/rejeitados");

	return 0;
}
