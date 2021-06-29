#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

int shmForSemaphore;
int shmForServer;
// queue for the blocked process in semaphore
struct Semaphore{
	int value;
	int fqSemaphore;
	int rqSemaphore;
	int queueSemaphore[1009];
};
struct Semaphore* S;

struct requestBuffer{
	int reqType;	// 1,2,3 for service type
	int PID;		// pid of client process
	int shmid_result_buffer;	// reference to result buffer
	int inputData[6];
};

struct queueBuffer{
	// front is index of the next request handled by server
	int front;
	// rear is the index at which client puts its next request
	int rear;
	// qSize denote the capacity of our queue
	int qSize;
	struct requestBuffer request[1000];
};
struct queueBuffer* queue;

void ctrl_c_Handler()
{
	printf("Clearing the shared memory ... \n");
	shmdt(S);
	shmctl(shmForSemaphore,IPC_RMID,NULL);
	shmdt(queue);
	shmctl(shmForServer,IPC_RMID,NULL);
	printf("\n Server program has been successfully terminated !! \n");
	exit(1);	
}

int main(){
	printf(" Server program has started \n");
	signal(SIGINT,ctrl_c_Handler);
	// Shared Memory for queue
				key_t keyForServer = ftok("buffer.txt",88);
				if(keyForServer<0){
					perror("errorKeyServerClient2: ");
					exit(0);
				}
				shmForServer= shmget(keyForServer,sizeof(struct queueBuffer),IPC_CREAT | 0666);
				if(shmForServer<0){
					perror("errorShmServerClient2: ");
					exit(0);
				}
				queue = (struct queueBuffer*)shmat(shmForServer,NULL,0);
				if(queue==(void*)-1){
					perror("errorQueueClient2: ");
					exit(0);
				}
			// queue 
				queue->front=-1;
				queue->rear=0;
				// qSize should be equal to size of request array in structure queueBuffer
				queue->qSize = 1000;

	// Shared memory for Semaphores

				key_t keyForSemaphore = ftok("buffer.txt",123);
				if(keyForSemaphore<0){
					perror("errorKeyForSeamaphoreclient3: ");
					exit(0);
				}
				shmForSemaphore = shmget(keyForSemaphore,sizeof(struct Semaphore),IPC_CREAT | 0666);
				if(shmForSemaphore<0){
					perror("errorShmForSemaphoreclient3 : ");
					exit(0);
				}
				S = (struct Semaphore*)shmat(shmForSemaphore,NULL,0);
				if(S==(void*)-1){
					perror("errorSemaphoreclient3 : ");
					exit(0);
				}
				S->value = 1;
				S->fqSemaphore = -1;
	// Shared memory for Semaphores end here
	// shared memory for queue created and initialized
	while(1){
		// if buffer is empty, wait for client 
			while( (queue->front==-1) || (queue->front==queue->rear) )sleep(1);
			struct requestBuffer* nextRequest = (struct requestBuffer*)malloc(sizeof(struct requestBuffer));
			*nextRequest = queue->request[queue->front];
			if(nextRequest->reqType==1){

				// this is a request of type 1 
				// input contains 6 integers
				char a1[20],a2[20],a3[20],a4[20],a5[20];
				char n[20];
				char PID[20],shmid[20];
				sprintf(a1,"%d",nextRequest->inputData[0]);
				sprintf(a2,"%d",nextRequest->inputData[1]);
				sprintf(a3,"%d",nextRequest->inputData[2]);
				sprintf(a4,"%d",nextRequest->inputData[3]);
				sprintf(a5,"%d",nextRequest->inputData[4]);
				sprintf(n,"%d",nextRequest->inputData[5]);
				sprintf(PID,"%d",nextRequest->PID);
				sprintf(shmid,"%d",nextRequest->shmid_result_buffer);
				int pid = fork();
				if(pid==0){
					execl("./service1","./service1",a1,a2,a3,a4,a5,n,PID,shmid,NULL);
				}
				// wait(NULL);
				queue->front = (queue->front + 1)%(queue->qSize);
			}
			else if(nextRequest->reqType==2){
				// request of type 2 
				// n as input, calculate its factorial
				char n[20];
				char PID[20],shmid[20];
				sprintf(n,"%d",nextRequest->inputData[0]);
				sprintf(PID,"%d",nextRequest->PID);
				sprintf(shmid,"%d",nextRequest->shmid_result_buffer);
				int pid=fork();
				if(pid==0){
					execl("./service2","./service2",n,PID,shmid,NULL);
				}
				// wait(NULL);
				queue->front = (queue->front + 1)%(queue->qSize);
			}
			else if(nextRequest->reqType==3){
				// request of type 3 
				// a,b as input calculate its gcd
				char a[20],b[20];
				char PID[20],shmid[20];
				sprintf(a,"%d",nextRequest->inputData[0]);
				sprintf(b,"%d",nextRequest->inputData[1]);
				sprintf(PID,"%d",nextRequest->PID);
				sprintf(shmid,"%d",nextRequest->shmid_result_buffer);
				int pid=fork();
				if(pid==0){
					execl("./service3","./service3",a,b,PID,shmid,NULL);
				}
				// wait(NULL);
				queue->front = (queue->front + 1)%(queue->qSize);
			}
	}
}
