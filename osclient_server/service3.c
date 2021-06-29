#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/ipc.h>
#include<sys/shm.h>
// gcd(a,b)
// shmid of client process is needed
// pid of client process is needed
// total 5 arguments
// ./service3 a,b pid, shmid

// function to calculate gcd of a and b
int gcd(int a,int b){
	if(a==0)return b;
	return gcd(b%a,a);
}

void my_handler(){}

int main(int argc,char* argv[]){
	signal(SIGUSR1,my_handler);
	// given a and b
	// calculate gcd of a and b
	if(argc!=5){
		printf("Invalid input in service3\n");
		exit(0);
	}
	int pid = atoi(argv[3]);
	printf("Service3 is started by client process having PID %d is started : --\n",pid);
	int a = atoi(argv[1]);
	int b = atoi(argv[2]);
	int result = gcd(a,b);

	// writing the result into client result buffer
	pid = atoi(argv[3]);
	int shmid = atoi(argv[4]);

	int* result_buffer_client = (int*)shmat(shmid,NULL,0);
	if(result_buffer_client==(void*)-1){
		perror("service3: ");
		exit(0);
	}
	*result_buffer_client = result;
	shmdt(result_buffer_client);
	kill(pid,SIGUSR1);
	printf("Service3 started by client process having PID %d is completed !! : --\n",pid);
}