#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define PRODUCERS 2
#define CONSUMERS 1
#define MAX_N 100
#define MAX_SIZE 10

int* cur_size;
int* consumer_sum;
int* producer_buffer[MAX_SIZE];
int* producer_ptr;
int* consumer_ptr;
int* prod_cons_counter;
int* limit_prod;
int mem_ctr = 0;

sem_t* producer_sem;
sem_t* consumer_sem;
sem_t* size_sem;
sem_t* count_cur_size_consumer;
sem_t* count_cur_size_producer;

sem_t* count_prod_cons;
sem_t* fin_count;

void producer()
{
    printf("Producer started\n");
    while(1) {
        printf("p waiting\n");
        sem_wait(size_sem);
        printf("p waited\n");
        while (*cur_size == MAX_SIZE) {
            sem_post(size_sem);
            sem_wait(size_sem);
        }
        (*cur_size)++;

        sem_wait(producer_sem); 
        *producer_buffer[*producer_ptr] = (rand() % 10) + 1;
        (*producer_ptr)++;
        (*producer_ptr) %= MAX_SIZE;
        (*limit_prod)--;
        if (*limit_prod < 0) {
            break;
        }

        sem_wait(count_prod_cons);
        (*prod_cons_counter)++;
        if ((*prod_cons_counter) >= MAX_N * 2) {
            sem_post(fin_count);
            break;
        }
        sem_post(count_prod_cons);

        sem_post(producer_sem); 

        sem_post(size_sem);
    }
}

void consumer()
{
    printf("Consumer started\n");
    while (1) {
        sem_wait(size_sem);
        while (*cur_size == 0) {
            sem_post(size_sem);
            sem_wait(size_sem);
        }
        (*cur_size)--;

        sem_wait(consumer_sem);
        (*consumer_sum) += *producer_buffer[*consumer_ptr];
        (*consumer_ptr)++;
        (*consumer_ptr) %= MAX_SIZE;
        printf("Current sum = %d\n", *consumer_sum);

        sem_wait(count_prod_cons);
        (*prod_cons_counter)++;
        if (*prod_cons_counter >= MAX_N * 2) {
            sem_post(fin_count);
            break;
        }
        sem_post(count_prod_cons);

        sem_post(consumer_sem);

        sem_post(size_sem);

    }
}

int* init_mem() {
    key_t shmkey = ftok("/dev/null", mem_ctr++); /* valid directory name and a number */
    int shmid = shmget(shmkey, sizeof(int), 0644 | IPC_CREAT);
    if (shmid < 0) { /* shared memory error check */
        perror("shmget\n");
        exit(1);
    }
    return (int*)shmat(shmid, NULL, 0); /* attach p to shared memory */
}

int main(int argc, char* argv[])
{
    producer_sem = sem_open("pSem", O_CREAT | O_EXCL, 0644, 1);
    consumer_sem = sem_open("cSem", O_CREAT | O_EXCL, 0644, 1);
    size_sem = sem_open("sSem", O_CREAT | O_EXCL, 0644, 1);

    count_prod_cons = sem_open("cpcSem", O_CREAT | O_EXCL, 0644, 1);
    fin_count = sem_open("finSem", O_CREAT | O_EXCL, 0644, 1);

    consumer_sum = init_mem();
    *consumer_sum = 0;

    cur_size = init_mem();
    *cur_size = 0;
    producer_ptr = init_mem();
    *producer_ptr = 0;
    consumer_ptr = init_mem();
    *consumer_ptr = 0;
    prod_cons_counter = init_mem();
    *prod_cons_counter = 0;
    limit_prod = init_mem();
    *limit_prod = MAX_N;

    for (int i = 0; i < MAX_SIZE; i++) {
        producer_buffer[i] = init_mem();
        *producer_buffer[i] = 0;
    }

    printf("Reached\n");

    pid_t pid;
    for (int i = 0; i < PRODUCERS; i++) {
        pid = fork();
        if (pid < 0) {
            printf("Fork error.\n");
        }
        if (pid == 0) {
            break;
        }
    }

    if (pid != 0) {
        consumer();
    } else {
        producer();
    }

    printf("Reached Final\n");

    sem_wait(fin_count);
    printf("### consumer_sum final value = %d ###\n",
        *consumer_sum);

    sem_unlink("pSem");
    sem_unlink("cSem");
    sem_unlink("sSem");
    sem_unlink("cpcSem");
    sem_unlink("finSem");
    sem_close(producer_sem);
    sem_close(consumer_sem);
    sem_close(size_sem);
    sem_close(count_prod_cons);
    sem_close(fin_count);
}
