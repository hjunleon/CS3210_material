/*******************************************************************
 * ex789-prod-con-threads.cpp
 * Producer-consumer synchronisation problem in C++
 *******************************************************************/



#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <semaphore.h>

#define MAX_SIZE 10

constexpr int PRODUCERS = 2;
constexpr int CONSUMERS = 1;

//int producer_buffer[10];
int *p[MAX_SIZE];
int mem_counter = 0;
//constexpr const char *SEMAPHORE_NAME = "pSem";
sem_t* s_sem;// = sem_open( "spaceSem", O_CREAT | O_EXCL, 0644, MAX_SIZE);
sem_t* p_sem;// = sem_open( "producerSem", O_CREAT | O_EXCL, 0644, 1); //producer

//int* producer_buffer[MAX_SIZE];
int *p_idx;

int consumer_sum = 0;
int c_idx = 0;

void producer()
{
	// Write producer code here
	int space = 0;	
	while(true){
		//printf("Checking if full\n");
		sem_wait(s_sem);
		sem_getvalue(s_sem, &space);
                printf("Producer decrease space to %d\n", space);

                //if(space == MAX_SIZE) continue;
		//printf("Acquiring lock\n");
		sem_wait(p_sem);
		int to_add = (rand() % 9) + 1;
		printf("p idx: %d has %d\n", *p_idx, to_add);
		*p[*p_idx] = to_add;
		(*p_idx)++;
		(*p_idx) %= MAX_SIZE;
		sem_post(p_sem);
		
	}
}

void consumer()
{
	// Write consumer code here
	int space = 0;
	while (true){
		
		sem_getvalue(s_sem, &space);
		printf("Consumer see space %d\n", space);
		
		if(space == MAX_SIZE) continue;
		
		sem_wait(p_sem);
                
		sem_post(s_sem);
		
		consumer_sum += *p[c_idx];
	       	printf("consumer_sum: %d\n", consumer_sum);
		c_idx++;
		c_idx %= MAX_SIZE;	
		sem_post(p_sem);
	}
}
int* init_mem() {
    key_t shmkey = ftok("/dev/null", mem_counter++); /* valid directory name and a number */
    int shmid = shmget(shmkey, sizeof(int), 0644 | IPC_CREAT);
    if (shmid < 0) { /* shared memory error check */
        perror("shmget\n");
        exit(1);
    }
    return (int*)shmat(shmid, NULL, 0); /* attach p to shared memory */
}
int main(int argc, char *argv[])
{
	sem_unlink("spaceSem");
	sem_unlink("producerSem");
	
	s_sem = sem_open("spaceSem", O_CREAT | O_EXCL, 0644, MAX_SIZE);	
	p_sem = sem_open("producerSem", O_CREAT | O_EXCL, 0644, 1);
	if (s_sem == SEM_FAILED || p_sem == SEM_FAILED){
		printf("can't create sem! %d %d\n", s_sem, p_sem);

		fprintf(stderr, "sem error: %s\n", strerror(errno));
		//strerror(errno);
		exit(1);
	}
	
	for (int i = 0; i < MAX_SIZE; i++) {
        	p[i] = init_mem();
        	*p[i] = 0;
    	}
	
	p_idx = init_mem();
	*p_idx = 0;
	int t1;
	
	for (t1 = 0; t1 < PRODUCERS; t1++)
	{
		printf("Main: creating producer %d\n", t1);
		if ( fork () == 0) producer();
		
	}

	consumer();

}
