#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/ipc.h>
#include<sys/shm.h>

// shmid of client process is needed
// pid of client process is needed
// total 9 arguments
// ./service1 a1,a2,a3,a4,a5,n pid shmid 

void my_handler(){}

int main(int argc,char* argv[]){
	if(argc!=9){
		printf("Invalid Input in service1 ! \n");
		exit(0);
	}
	signal(SIGUSR1,my_handler);
	int pid = atoi(argv[7]);
	printf("Service1 is started by client process having PID %d : --\n",pid); 

	int arr[5];
	int n = atoi(argv[6]);
	int result=-1;
	for(int i=1;i<6;++i){
		arr[i-1] = atoi(argv[i]);
	}
	for(int i=0;i<5;++i){
		if(arr[i]==n){
			// number found
			result = i;
			break;
		}
	}

	// writing the result into client result buffer
	pid = atoi(argv[7]);
	int shmid = atoi(argv[8]);
	int* result_buffer_client = (int*)shmat(shmid,NULL,0);
	if(result_buffer_client==(void*)-1){
		perror("service1: ");
		exit(0);
	}
	*result_buffer_client = result;
	shmdt(result_buffer_client);
	kill(pid,SIGUSR1);
	printf("Service1 started by client process having PID %d is completed !! --\n",pid);
}
