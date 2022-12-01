/*******************************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************************/
#include <iostream>
using namespace std;
#include "helper.h"
#include "queue.h"


Queue *circQ;
int n_of_jobs;
const int MAX_WAIT = 20;
struct timespec time_s;

/* SEMAPHORES */
enum Semaphore{EMPTY, FULL, MUTEX};
const int semaphores_no =3;
// Create semaphores
int semID = sem_create(SEM_KEY, semaphores_no);

void *producer (void *id);
void *consumer (void *id);

/******************************** MAIN ********************************/

int main (int argc, char **argv) {
  
  // check if right number of arguments provided
  if(argc != 5){
  cerr << "Number of arguments must be four!" << endl;
  return 0;
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

  // initialise semaphores
  sem_init(semID, MUTEX, 1);
  sem_init(semID, EMPTY, queue_size);
  sem_init(semID, FULL, 0);
  
  // create producer threads
  pthread_t producerid[num_producers];
  int prod_id[num_producers];
  for(int n=0; n<num_producers; n++){
    prod_id[n]=n+1;
    if((pthread_create(&producerid[n], NULL, producer, (void*)&prod_id[n])==0)){
      // successful creation of thread
    }
    else{
      // unsuccessful creation of thread
      perror("Producer thread creation unsuccessful\n");
    }
  }

  // create consumer threads
  pthread_t consumerid[num_consumers];
  int con_id[num_consumers];
  for(int n=0; n<num_consumers; n++){
    con_id[n]=n+1;
    if((pthread_create(&consumerid[n], NULL, consumer, (void*)&con_id[n])==0)){
      // successful creation of thread
    }
    else{
      // unsuccessful creation of thread
      perror("Consumer thread creation unsuccessful\n");
    }
  }

  // join producer and consumer threads
  for (int n=0; n<num_producers; n++){
    if((pthread_join(producerid[n], NULL))==0){
      // successful join
    }
    else{
      // unsuccessful  join
      perror("Producer thread join unsuccessful\n");
    }
  }
  for (int n=0; n<num_consumers; n++){
    if((pthread_join(consumerid[n], NULL))==0){
      // successful join
    }
    else{
      // unsuccessful  join
      perror("Consumer thread join unsuccessful\n");
    }
  }
  sem_close(semID);
  delete circQ;
  printf("\nDelete all semaphores using the Unix command 'ipcrm -a'\n");
  return 0;
}

/****************************** PRODUCER ******************************/

void *producer (void *pnumber) {

  int wait_return;
  int job_n=0;

  while(job_n<n_of_jobs){
    // set the number of seconds of the time_s structure to MAX_WAIT (20s)
    time_s.tv_sec=MAX_WAIT;
    // Produce a job with duration between 1-10 seconds
    int job_duration = rand()%10 +1;
    // wait if queue is not empty (wait maximum of 20s)
    while((wait_return=sem_timed_wait(semID, EMPTY, &time_s))==-1 && 
           errno == EINTR)
      continue; // restart if signal handler interrupt

    // if timed_wait unsuccessful:
    if((wait_return==-1)) { 
      if (errno == EAGAIN){ // time limit (20s) expired
        printf("Producer(%d)", *((int*)pnumber));
        printf("No slot in queue empty after 20s. Producer quitting!\n");
        pthread_exit((void*) 0);
      }
      else{
        printf("Make sure you have deleted previously used semaphores;");
        printf(" use the Unix command 'ipcrm -a' \n");
        pthread_exit((void*) 0);
      }
    }
    // if timed_wait successful:
    else{
      sem_wait(semID, MUTEX);
      /*----------- entering the critical section -----------*/
      // Add the job to the circular queue
      circQ->addElement(job_duration);
      printf("Producer(%d): Job id %d duration %d\n",\
      *((int*)pnumber), circQ->get_end(), circQ->get_element(circQ->get_end()));
      /*----------- leaving the critical section -----------*/
      sem_signal(semID, MUTEX);
      sem_signal(semID, FULL);
      job_n++;
    }
    // wait 1-5 seconds before adding another job
    sleep(rand()%5 +1);
  }
  printf("Producer(%d): No more jobs to generate.\n", *((int*)pnumber));
  pthread_exit((void*) 0);
}

/****************************** CONSUMER ******************************/

void *consumer (void *cnumber) {
  
  int wait_return;

  while(1){
    // set the number of seconds of the time_s structure to MAX_WAIT (20s)
    time_s.tv_sec=MAX_WAIT; 
    // wait if queue is not filled (wait maximum of 20s)
    while((wait_return = sem_timed_wait(semID, FULL, &time_s))==-1 && 
           errno == EINTR)
      continue; // restart if signal handler interrupt

    // if timed_wait unsuccessful:  
    if(wait_return==-1){
      if (errno == EAGAIN){ // time limit (20s) expired
        printf("Consumer(%d): No more jobs left.\n", *((int*)cnumber));
        pthread_exit((void*) 0);
      }
      else{
        printf("Make sure you have deleted previously used semaphores;");
        printf(" use the Unix command 'ipcrm -a' \n");
        pthread_exit((void*) 0);
      }
    }
    // if timed_wait successful:
    else{
      sem_wait(semID, MUTEX);
      /*----------- entering the critical section -----------*/
      // consume a job 
      int job_id = circQ->get_start();
      int job_consumed = circQ->deleteElement();
      printf("Consumer(%d): Job id %d executing sleep duration %d\n",\
      *((int*)cnumber), job_id, job_consumed);
      /*----------- leaving the critical section -----------*/
      sem_signal(semID, MUTEX);
      // sleep for time it takes to process the job 
      sleep(job_consumed);
      printf("Consumer(%d): Job id %d completed\n", *((int*)cnumber), job_id);
      sem_signal(semID, EMPTY);
    }
  }
  pthread_exit((void*) 0);
}
