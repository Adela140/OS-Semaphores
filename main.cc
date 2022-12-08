/*******************************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************************/
#include <iostream>
using namespace std;
#include "helper.h"
#include "queue.h"
#include <chrono>

/* GLOBAL VARIABLES */
Queue *circQ;
int n_of_jobs;
const int MAX_WAIT = 20;
struct timespec time_s;

/* SEMAPHORES */
enum Semaphore{EMPTY, FULL, MUTEX};
const int semaphores_no =3;
int semID;

void *producer (void *id);
void *consumer (void *id);


/******************************** MAIN ********************************/

int main (int argc, char **argv) {
  
  auto begin = std::chrono::high_resolution_clock::now(); // start clock
  
  // check if right number of arguments provided
  if(argc != 5){
    printf("Error: Number of arguments must be four!\n");
    exit(1);
  }

  // check if the arguments are positive
  for(int i=1; i<5; i++){
    if(check_arg(argv[i])<0){
      printf("Error: Arguments must be integers equal to or larger than 0!\n");
      exit(1);
    }
  }

  // store the command line arguments
  int queue_size = check_arg(argv[1]);
  n_of_jobs = check_arg(argv[2]);
  int num_producers = check_arg(argv[3]);
  int num_consumers = check_arg(argv[4]);

  printf("\nTHE INPUTS: \nQueue size=%d ; Number of jobs per producer=%d ;\
  Number of producers=%d ; Number of consumers=%d \n \n",\
  queue_size, n_of_jobs, num_producers, num_consumers);

  // seed for rand determined with internal clock
  srand(time(NULL));

  // create circular queue (buffer) of size 'queue_size'
  circQ = new Queue(queue_size);

  // Create and initialise semaphores
  semID = sem_create(SEM_KEY, semaphores_no);
  if(semID==-1){
    printf("Error: Failed to create semaphore array.\n");
    printf("Make sure you have deleted previously used "
        "semaphores; use the Unix command 'ipcrm -a' \n");
    exit(1);
  }
  if(sem_init(semID, MUTEX, 1)==-1){
    printf("Error: Failed to initialise MUTEX semaphore.\n");
    exit(1);
  }
  if(sem_init(semID, EMPTY, queue_size)==-1){
    printf("Error: Failed to initialise EMPTY semaphore.\n");
    exit(1);
  }
  if(sem_init(semID, FULL, 0)==-1){
    printf("Error: Failed to initialise FULL semaphore.\n");
    exit(1);
  }
  
  // create producer threads
  pthread_t producerid[num_producers];
  int prod_id[num_producers];
  for(int n=0; n<num_producers; n++){
    prod_id[n]=n+1;
    if(pthread_create(&producerid[n], NULL, producer, (void*)&prod_id[n])!=0){
      // unsuccessful creation of thread
      printf("Error: Failed to create producer thread.\n");
      exit(1);
    }
  }

  // create consumer threads
  pthread_t consumerid[num_consumers];
  int con_id[num_consumers];
  for(int n=0; n<num_consumers; n++){
    con_id[n]=n+1;
    if(pthread_create(&consumerid[n], NULL, consumer, (void*)&con_id[n])!=0){
      // unsuccessful creation of thread
      printf("Error: Failed to create consumer thread.\n");
      exit(1);
    }
  }

  // join producer and consumer threads
  for (int n=0; n<num_producers; n++){
    if((pthread_join(producerid[n], NULL))!=0){
      // unsuccessful  join
      printf("Error: Failed to join producer thread.\n");
      exit(1);
    }
  }
  for (int n=0; n<num_consumers; n++){
    if((pthread_join(consumerid[n], NULL))!=0){
      // unsuccessful  join
      printf("Error: Failed to join consumer thread.\n");
      exit(1);
    }
  }
  // clean up 
  if(sem_close(semID)==-1){
    // unsuccessful clean up
    printf("Error: Failed to close semaphore array.\n");
  }
  delete circQ;

  // time taken 
  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed=std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin);
  printf("\nTOTAL TIME TAKEN: %.2fs\n\n", elapsed.count()*1e-9);
  return 0;
}

/****************************** PRODUCER ******************************/

void *producer (void *pnumber) {
  
  int job_n=0;

  while(job_n<n_of_jobs){
    // wait 1-5 seconds before adding a job
    sleep(rand()%5 +1);

    // Produce a job with duration between 1-10 seconds
    int job_duration = rand()%10 +1;   

    // set the number of seconds of the time_s structure to MAX_WAIT (20s)
    time_s.tv_sec=MAX_WAIT;
    
    // wait if queue is not empty (wait maximum of 20s)
    // if timed_wait unsuccessful:
    if(sem_timed_wait(semID, EMPTY, &time_s)==-1) { 
      if (errno == EAGAIN){ // time limit (20s) expired
        fprintf(stderr, "Producer(%d): No slot in queue after 20s."
        " Producer quitting!\n",*((int*)pnumber));
        pthread_exit(0);
      }
      else{
        perror("Error");
        fprintf(stderr,"Error: Make sure you have deleted previously used "
        "semaphores; use the Unix command 'ipcrm -a' \n");
        exit(1);
      }
    }
    // if timed_wait successful:
    else{
      sem_wait(semID, MUTEX);
      /*----------- entering the critical section -----------*/
      // Add the job to the circular queue
      int job_id = circQ->get_end();
      circQ->addElement(job_duration);
      fprintf(stderr, "Producer(%d): Job id %d duration %d\n",\
      *((int*)pnumber), job_id, job_duration);
      /*----------- leaving the critical section -----------*/
      sem_signal(semID, MUTEX);
      sem_signal(semID, FULL);
      job_n++;
    }
  }
  fprintf(stderr,"Producer(%d): No more jobs to generate.\n", *((int*)pnumber));
  pthread_exit(0);
}

/****************************** CONSUMER ******************************/

void *consumer (void *cnumber) {

  while(1){
    // set the number of seconds of the time_s structure to MAX_WAIT (20s)
    time_s.tv_sec=MAX_WAIT; 

    // wait if queue is not filled (wait maximum of 20s)
    // if timed_wait unsuccessful:  
    if(sem_timed_wait(semID, FULL, &time_s)==-1){
      if (errno == EAGAIN){ // time limit (20s) expired
        fprintf(stderr, "Consumer(%d): No more jobs left.\n", *((int*)cnumber));
        pthread_exit(0);
      }
      else{
        perror("Error:");
        fprintf(stderr,"Error: Make sure you have deleted previously used "
        "semaphores; use the Unix command 'ipcrm -a' \n");
        exit(1);
      }
    }
    // if timed_wait successful:
    else{
      sem_wait(semID, MUTEX);
      /*----------- entering the critical section -----------*/
      // consume a job 
      int job_id = circQ->get_start();
      int job_consumed = circQ->deleteElement();
      fprintf(stderr, "Consumer(%d): Job id %d executing sleep duration %d\n",\
      *((int*)cnumber), job_id, job_consumed);
      /*----------- leaving the critical section -----------*/
      sem_signal(semID, MUTEX);
      sem_signal(semID, EMPTY);
      // sleep for time it takes to process the job 
      sleep(job_consumed);
      fprintf(stderr, "Consumer(%d): Job id %d completed\n",\
       *((int*)cnumber), job_id);
    }
  }
  pthread_exit(0);
}
