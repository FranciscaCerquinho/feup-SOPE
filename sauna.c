#include "pedido.h"

#define NUM_CHARS_FILE_NAME 100
#define NUM_MAX_CLIENTES 10000


pthread_cond_t no_seats = PTHREAD_COND_INITIALIZER;
int seats;
pthread_mutex_t seats_sem;
int record;
pthread_mutex_t record_sem;
int served_f=0;
int served_m=0;
struct timespec ts;
struct timespec tInitial;

void* order_processing(void* pedido){
    struct request inSauna = *((struct request *) pedido);
    usleep(inSauna.timeReq*1000);
    pthread_mutex_lock(&record_sem);
    timespec_get(&ts, TIME_UTC);
    dprintf(record,"%8.2f - %d - %ld - %4d: %c - %4d - SERVIDO\n",(ts.tv_sec - tInitial.tv_sec)*1000+(ts.tv_nsec - tInitial.tv_nsec)*0.000001,getpid(),pthread_self(),inSauna.serial_number,inSauna.gender, inSauna.timeReq);
    if(inSauna.gender=='F'){
        served_f++;
    }else{
        served_m++;
    }
    pthread_mutex_unlock(&record_sem);
    pthread_mutex_lock(&seats_sem);
    seats++;
    pthread_cond_broadcast(&no_seats);
    pthread_mutex_unlock(&seats_sem);
    return NULL;
}

int main(int argc, char** argv){
    timespec_get(&tInitial, TIME_UTC);
    //Variable validation
    if(argc != 2){
        printf("Usage: sauna <n. lugares>\n");
        return 1;
    }
    //Initialization of mutexes and condition variables
    pthread_mutex_init(&seats_sem,NULL);
    pthread_mutex_init(&record_sem,NULL);
    pthread_cond_init(&no_seats,NULL);
    //Initialization of the number of seats
    int nSeats = atoi(argv[1]);
    pthread_mutex_lock(&seats_sem);
    seats=nSeats;
    pthread_mutex_unlock(&seats_sem);

    //Creation and opening of fifos
    if(mkfifo("/tmp/entrada",0660)){
        perror("sauna");
        printf("Erro ao criar FIFO - /tmp/entrada\n");
        return 1;
    }
    int fifo_entrada;
    if((fifo_entrada = open("/tmp/entrada",O_RDONLY)) == -1){
        perror("sauna");
        printf("Erro ao abrir FIFO - /tmp/entrada\n");
        return 1;
    }
    int fifo_rejeitados;
    do{
        fifo_rejeitados= open("/tmp/rejeitados",O_WRONLY);
    }while(fifo_rejeitados==-1);

    //Creation and opening of log file
    char nameOfFile[NUM_CHARS_FILE_NAME];
    sprintf(nameOfFile, "%s%d","/tmp/bal.", getpid());
    pthread_mutex_lock(&record_sem);
    if((record = open(nameOfFile,O_WRONLY | O_CREAT | O_EXCL,0666)) == -1){
        perror("sauna");
        pthread_mutex_unlock(&record_sem);
        printf("Erro ao abrir FILE - %s\n",nameOfFile);
        return 1;
    }
    pthread_mutex_unlock(&record_sem);

    //Order Processing
    char gender='G';
    struct request received[NUM_MAX_CLIENTES];
    pthread_t tid[NUM_MAX_CLIENTES];
    int i=0;
    //Variables for statistics
    int received_f=0;
    int received_m=0;
    int rejected_f=0;
    int rejected_m=0;

    while(i<NUM_MAX_CLIENTES){
        //Receive request
        if(read(fifo_entrada,&(received[i]),sizeof(received[0]))==0)break;
        pthread_mutex_lock(&record_sem);
        timespec_get(&ts, TIME_UTC);
        dprintf(record,"%8.2f - %d - %ld - %4d: %c - %4d - RECEBIDO\n",(ts.tv_sec - tInitial.tv_sec)*1000+(ts.tv_nsec - tInitial.tv_nsec)*0.000001,getpid(),pthread_self(),received[i].serial_number,received[i].gender, received[i].timeReq);
        if(received[i].gender=='F'){
            received_f++;
        }else{
            received_m++;
        }
        pthread_mutex_unlock(&record_sem);

        //If nobody is in the Sauna, update gender
        pthread_mutex_lock(&seats_sem);
        if(seats==nSeats){
            pthread_mutex_unlock(&seats_sem);
            gender=received[i].gender;
        }else{
            pthread_mutex_unlock(&seats_sem);
        }

        //If in the sauna are people of another gender, reject
        if(gender!=received[i].gender){
             pthread_mutex_lock(&record_sem);
             timespec_get(&ts, TIME_UTC);
             dprintf(record,"%8.2f - %d - %ld - %4d: %c - %4d - REJEITADO\n",(ts.tv_sec - tInitial.tv_sec)*1000+(ts.tv_nsec - tInitial.tv_nsec)*0.000001,getpid(),pthread_self(),received[i].serial_number,received[i].gender, received[i].timeReq);
             if(received[i].gender=='F'){
                rejected_f++;
             }else{
                rejected_m++;
             }
             pthread_mutex_unlock(&record_sem);
             write(fifo_rejeitados,&(received[i]),sizeof(received[0]));
        }else
        {
            //If there are seats, enter the sauna
            while(1){
                pthread_mutex_lock(&seats_sem);
                if(seats>0){
                    seats--;
                    pthread_mutex_unlock(&seats_sem);
                    pthread_create(&(tid[i]), NULL, &order_processing, &(received[i]));
                    break;
                }else{
                    //Waits for seats
                    pthread_cond_wait(&no_seats,&seats_sem);
                    pthread_mutex_unlock(&seats_sem);
                }
            }
            i++;
        }
    }
    //Wait for the threads to complete
    int num_tids=i;
    for(i=0;i<num_tids;i++){
        pthread_join(tid[i],NULL);
    }

    //Print statistics
    pthread_mutex_lock(&record_sem);
    dprintf(STDOUT_FILENO,"N. recebidos:\n%d, %d, %d\nN. rejeitados:\n%d, %d, %d\nN. servidos:\n%d, %d, %d\n"
            ,(received_f+received_m),received_f,received_m
            ,(rejected_f+rejected_m),rejected_f,rejected_m
            ,(served_f+served_m),served_f,served_m);
    pthread_mutex_unlock(&record_sem);

    //Close the variables
    close(fifo_rejeitados);
    pthread_mutex_lock(&record_sem);
    close(record);
    pthread_mutex_unlock(&record_sem);
    pthread_mutex_destroy(&record_sem);
    pthread_mutex_destroy(&seats_sem);
    pthread_cond_destroy(&no_seats);
    close(fifo_entrada);
    unlink("/tmp/entrada");
    return 0;
}
