/*******************************************************************
 * ex789-prod-con-threads.cpp
 * Producer-consumer synchronisation problem in C++
 *******************************************************************/

#include <cstdio>
#include <cstdlib>
#include <pthread.h>

constexpr int PRODUCERS = 2;
constexpr int CONSUMERS = 1;

int consumer_sum = 0;
int producer_buffer[10];
int free_space = 10;


int producer_idx = 0;
int consumer_idx = 0;

pthread_mutex_t p_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t item_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t space_cond = PTHREAD_COND_INITIALIZER;
//pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *producer(void *threadid)
{
	// Write producer code here
	while(true){
		int to_add = (rand() % 9) + 1;
		//pthread_cond_wait(&space_cond, &p_lock);
		pthread_mutex_lock(&p_lock);
		if(free_space > 0){
		// if (producer_idx != consumer_idx){
			producer_buffer[producer_idx] = to_add;
			//printf("p idx: %d has %d\n", producer_idx, to_add);
			producer_idx++;
			producer_idx %= 10;
			free_space--;
			pthread_cond_signal(&item_cond);
		
		}
		pthread_mutex_unlock(&p_lock);
	}
}	

void *consumer(void *threadid)
{
	// Write consumer code here
	while(true){
		
		pthread_cond_signal(&item_cond);
		pthread_mutex_lock(&p_lock);
			
		if( free_space < 10 ){
			consumer_sum += producer_buffer[consumer_idx];
			printf("consumer_sum: %d\n", consumer_sum);
			consumer_idx++;
			consumer_idx %= 10;
			free_space++;
		}
		pthread_mutex_unlock(&p_lock);
		//pthread_cond_signal(&space_cond);
	}
}

int main(int argc, char *argv[])
{
	pthread_t producer_threads[PRODUCERS];
	pthread_t consumer_threads[CONSUMERS];
	int producer_threadid[PRODUCERS];
	int consumer_threadid[CONSUMERS];

	int rc;
	int t1, t2;
	for (t1 = 0; t1 < PRODUCERS; t1++)
	{
		int tid = t1;
		producer_threadid[tid] = tid;
		printf("Main: creating producer %d\n", tid);
		rc = pthread_create(&producer_threads[tid], NULL, producer,
							(void *)&producer_threadid[tid]);
		if (rc)
		{
			printf("Error: Return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	for (t2 = 0; t2 < CONSUMERS; t2++)
	{
		int tid = t2;
		consumer_threadid[tid] = tid;
		printf("Main: creating consumer %d\n", tid);
		rc = pthread_create(&consumer_threads[tid], NULL, consumer,
							(void *)&consumer_threadid[tid]);
		if (rc)
		{
			printf("Error: Return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	pthread_exit(NULL);

	/*
					some tips for this exercise:

					1. you may want to handle SIGINT (ctrl-C) so that your program
									can exit cleanly (by killing all threads, or just calling
		 exit)

					1a. only one thread should handle the signal (POSIX does not define
									*which* thread gets the signal), so it's wise to mask out the
		 signal on the worker threads (producer and consumer) and let the main
		 thread handle it
	*/
}
