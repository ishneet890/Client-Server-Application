#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/ipc.h>
#include<sys/shm.h>
// fac(n)
// shmid of client process is needed
// pid of client process is needed
// total 4 arguments
// ./service1 n pid shmid

void my_handler(){}

int main(int argc,char* argv[]){
	signal(SIGUSR1,my_handler);
	// fac(n)
	if(argc!=4){
		printf("Invalid input in service2 \n");
		exit(0);
	}
	int pid = atoi(argv[2]);
	printf("Service2 is started by client process having PID %d : --\n",pid);
	int n = atoi(argv[1]);
	long long result = 1;
	for(int i=2;i<=n;++i){
		result = result*i;
	}
	
	// writing the result into client result buffer
	pid = atoi(argv[2]);
	int shmid = atoi(argv[3]);

	int* result_buffer_client = (int*)shmat(shmid,NULL,0);
	if(result_buffer_client==(void*)-1){
		perror("service1: ");
		exit(0);
	}
	*result_buffer_client = result;
	shmdt(result_buffer_client);
	kill(pid,SIGUSR1);
	printf("Service2 started by client process having PID %d is completed !! : --\n",pid);
}