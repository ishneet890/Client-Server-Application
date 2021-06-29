#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<stdbool.h>


// structure to hold all the necessary data required for a request by client
struct requestBuffer{
	int reqType;	// 1,2,3 for service type
	int PID;		// pid of client process
	// reference to result buffer
	// service process will use this to share the result with client
	int shmid_result_buffer;	
	int inputData[6];	// input data provided by user for computation
};


// queue denotes our shared request queue which contains the request of all clients
// it will be shared between : all the clients and server 
struct queueBuffer{
	// front is index of the next request handled by server
	int front;
	// rear is the index at which client puts its next request
	int rear;
	// qSize denote the capacity of our queue
	int qSize;
	struct requestBuffer request[1000];
};

// structure defined for semaphore implementation
struct Semaphore{
	int value;	// integer value of semaphore
	// fqSemaphore stands for : front Queue for Semaphore
	// similar rqSemaphore stands for : rear Queue for Semaphore
	// these are for queue implementation
	// blocked process are pushed into queue (to achieve mutual exclusion)
	int fqSemaphore;
	int rqSemaphore;
	int queueSemaphore[1009];
};

// DOWN and up function are part of semaphore implementation
void DOWN(struct Semaphore* S){
	S->value = S->value - 1;
	// The print statement commented below can be used to see 
	// which client is calling down function with what semaphore value
	printf("DOWN called by Client3 \n");
	// using these print statements we can verify the working of semaphore
	if(S->value<0){
		// when client3 tries to access critical section, but other client is already in critical section
		// client3's pid is pushed into the semaphore queue
		// client3 is then paused
		// later when other client comes out from critical section, 
		// it sends a kill signal to wake up the client3 using pid of client3
		printf("Client 3 is being locked: \n");

		// add the pid of process into semaphore queue
		S->queueSemaphore[S->rqSemaphore] = getpid();
		S->rqSemaphore = (S->rqSemaphore + 1)%(1009);
		if(S->fqSemaphore==-1)S->fqSemaphore=0;
		pause();
		printf("Client 3 is being unblocked : \n");
	}
	else
		return;
}

void UP(struct Semaphore* S){
	S->value = S->value + 1;
	printf("UP is called by client3\n");
	if(S->value<=0){
		// Client3 has successfully accessed critical section, now it is leaving 
		// so if any other process is in queue to access the critical section
		// client3 sends a kill signal to allow access to critical section
		int pid = S->queueSemaphore[S->fqSemaphore];
		S->fqSemaphore = (S->fqSemaphore + 1)%1009 ;
		kill(pid,SIGUSR1);
	}
	else 
		return;
}

void my_handler(){}
int main(){
	printf("Client3 with PID %d has started \n",getpid());
	signal(SIGUSR1,my_handler);

	// Shared Memory for request queue
		// this request queue will store the request of multiple clients
		// and is shared between server and all the clients
				key_t keyForServer = ftok("buffer.txt",88);
				if(keyForServer<0){
					perror("errorKeyServerClient3: ");
					exit(0);
				}
				int shmForServer= shmget(keyForServer,sizeof(struct queueBuffer),IPC_CREAT | 0666);
				if(shmForServer<0){
					perror("errorShmServerClient3: ");
					exit(0);
				}
				struct queueBuffer* queue = (struct queueBuffer*)shmat(shmForServer,NULL,0);
				if(queue==(void*)-1){
					perror("errorQueueClient3: ");
					exit(0);
				}
			// queue 
	// shared memory for request queue created

	// Shared memory for Semaphores
		// the semaphore data : (blockedQueue and semaphore value) is shared 
				key_t keyForSemaphore = ftok("buffer.txt",123);
				if(keyForSemaphore<0){
					perror("errorKeyForSeamaphoreclient3: ");
					exit(0);
				}
				int shmForSemaphore = shmget(keyForSemaphore,sizeof(struct Semaphore),IPC_CREAT | 0666);
				if(shmForSemaphore<0){
					perror("errorShmForSemaphoreclient3: ");
					exit(0);
				}
				struct Semaphore* S = (struct Semaphore*)shmat(shmForSemaphore,NULL,0);
				if(S==(void*)-1){
					perror("errorSemaphoreclient3: ");
					exit(0);
				}
	// Shared memory for Semaphores end here

	// creating shared memory for result buffer
		// service will place the computed value into this result buffer
		// we will pass the shmid of this result buffer 
		// so that service can attach itself to it , write the data, then detach itself
				key_t keyForResult = ftok("buffer.txt",33);
				if(keyForResult<0){
					perror("errorKeyResultClient3: ");
					exit(0);
				}
				int shmForResult = shmget(keyForResult,sizeof(int),IPC_CREAT | 0666);
				if(shmForResult<0){
					perror("errorShmForResultClient3: ");
					exit(0);
				}
				// since the resutl for all the three services we have choosen is an integer
				// we have created a shared memory of integer type
				int* result3 = (int*)shmat(shmForResult,NULL,0);
				if(result3==(void*)-1){
					perror("errorResultClient3: ");
					exit(0);
				}
	// shared memory "result3" is created


	// taking input from user
		// newRequest will store the inputdata, PID, shmid of the current request,
		// later this request will be enqueued into the queue for server process

				struct requestBuffer* newRequest = (struct requestBuffer*)malloc(sizeof(struct requestBuffer));
				// storing pid of client3 process
				// this pid will be used by service process to send a kill signal 
				newRequest->PID = getpid();

				// storing shmid of the result buffer
				// service process will use this to attach itself to result buffer and write the result
				// client will then receive the kill signal, denoting result has been computed
				// and it will print the data and terminate
				newRequest->shmid_result_buffer = shmForResult;

		// Ask the user for type of service it wants : - 
		printf("Enter type of service : \n");
		int type;
		scanf("%d",&type);
		if(type>=1 && type<=3){
			if(type==1){
				newRequest->reqType = 1;
				// array of size 5, find index of n 
				printf("Enter 5 Integers : \n");
				// we take input data from user 
				for(int i=0;i<5;++i){
					scanf("%d",&(newRequest->inputData[i]));
				}
				printf("Enter the number to be searched : \n");
				scanf("%d",&newRequest->inputData[5]);

				// newRequest has taken the request as input from user, now time to enqueue it
				// if buffer is full wait for server
					while( (queue->rear==(queue->qSize-1) && queue->front==0)  || (queue->rear)==(queue->front-1))sleep(2);
				// enqueue the request

					// now client3 will try to access the request queue,
					// since multiple processes can access this request queue
					// Mutual Exclusion required here /////////////////////////////
					
							// DOWN is called 
							DOWN(S);
							for(long long int i=0;i<1e10;++i){
								// client3 program acceses the queue for a very small amount of time 
								// so it is not easy to see the working of semaphore
								// So 
								// we can use this loop to increase the access time of client in critical section
								// and then we will see clients processes being blocked , denoting mutual exclusiveness
							}

							// this is the queue implementation in C 
							// nextIndex denote the next free index in queue where request needs to be enqued
							int nextIndex = queue->rear;
							queue->request[nextIndex] = *newRequest;
							queue->rear = (queue->rear+1)%(queue->qSize);
							// if queue is empty
							if(queue->front==-1)queue->front=0;		

							// UP is called
							UP(S);
					////////////////////////////////////////////////////////////
					pause();
					if((*result3)==-1){
						printf("%d is not present in the given array\n",newRequest->inputData[5]);
					}
					else
						printf("%d is found at index %d\n",newRequest->inputData[5],*result3);
			}
			else if(type==2){
				newRequest->reqType = 2;
				// calculate factorial
				printf("Enter n :\n");
				scanf("%d",&(newRequest->inputData[0]));
				// newRequest has taken the request as input from user, now time to enqueue it
				// if buffer is full wait for server
				while( (queue->rear==(queue->qSize-1) && queue->front==0)  || (queue->rear)==(queue->front-1))sleep(2);
				// enqueue the request
					// Mutual Exclusion required here /////////////////////////////
					
							// DOWN is called 
							DOWN(S);
							for(long long int i=0;i<1e10;++i){
							}

							// this is the queue implementation in C 
							// nextIndex denote the next free index in queue where request needs to be enqued
							int nextIndex = queue->rear;
							queue->request[nextIndex] = *newRequest;
							queue->rear = (queue->rear+1)%(queue->qSize);
							// if queue is empty
							if(queue->front==-1)queue->front=0;		

							// UP is called
							UP(S);
					////////////////////////////////////////////////////////////
					pause();
					printf("Factorial of %d is %d\n",newRequest->inputData[0],*result3);
			}
			else if(type==3){
				newRequest->reqType = 3;
				// calculate gcd 
				printf("Enter a and b: \n");
				scanf("%d",&(newRequest->inputData[0]));
				scanf("%d",&(newRequest->inputData[1]));
				// newRequest has taken the request as input from user, now time to enqueue it
				// if buffer is full wait for server
				while( (queue->rear==(queue->qSize-1) && queue->front==0)  || (queue->rear)==(queue->front-1))sleep(2);
				// enqueue the request
					// Mutual Exclusion required here /////////////////////////////
					
							// DOWN is called 
							DOWN(S);
							for(long long int i=0;i<1e10;++i){
							}
							int nextIndex = queue->rear;
							queue->request[nextIndex] = *newRequest;
							queue->rear = (queue->rear+1)%(queue->qSize);
							// if queue is empty
							if(queue->front==-1)queue->front=0;		

							// UP is called
							UP(S);
					////////////////////////////////////////////////////////////
					pause();
					printf("gcd(%d,%d) = %d\n",newRequest->inputData[0],newRequest->inputData[1],*result3);
			}
		}
		else{
			printf("Invalid Request ! \n");
		}
		// client has successfully used the services required
		// now it will detach itself from the shared memories
		shmdt(queue);
		shmdt(result3);
		shmdt(S);

		// free the shared memory for result buffer created
		shmctl(shmForResult,IPC_RMID,NULL);
}
