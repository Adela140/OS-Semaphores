/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/
#include <iostream>
using namespace std;
#include "helper.h"
#include "queue.h"
#include "semaphore.h"


Queue circQ;
int n_of_jobs;
const int MAX_WAIT = 20;

/* SEMAPHORES */
//#define EMPTY   0
//#define FULL    1
//#define MUTEX   2 

enum Semaphore{EMPTY, FULL, MUTEX};

const int semaphores_no =3;

// Create semaphores
int semID = sem_create(SEM_KEY, semaphores_no);

void *producer (void *id);
void *consumer (void *id);

int main (int argc, char **argv) {
  // check if right number of arguments provided
    if(argc != 5){
    cerr << "Number of arguments must be four!" << endl;
    return 0;
    }

  // seed for rand determined with internal clock
  srand(time(NULL));

  // create circular queue (buffer)
  // size of the queue
  int queue_size = check_arg(argv[1]);
  cout<<"Size of queue:"<<queue_size<<endl;
  circQ.createQueue(queue_size);
  for(int i=0; i<queue_size; i++){
    cout<< circQ.get_element(i)<<endl;
  }
  cout<<circQ.get_start();
  cout<<circQ.get_end();

  // number of jobs per producers
  int jobs_per_producer = check_arg(argv[2]);
  n_of_jobs = jobs_per_producer;

  // initialise semaphores
  sem_init(semID, MUTEX, 1);
  sem_init(semID, EMPTY, queue_size);
  sem_init(semID, FULL, 0);

  // create producer threads
  // number of producers
  int num_producers = check_arg(argv[3]);
  pthread_t producerid[num_producers];
  int prod_id[num_producers];
  for(int n=0; n<num_producers; n++){
    prod_id[n]=n+1;
    if((pthread_create(&producerid[n], NULL, producer, (void*)&prod_id[n])==0)){
      // successful creation of thread
      //cout<<"Successfully created producer thread"<<endl;
    }
    else{
      // unsuccessful creation of thread
      perror("Producer thread creation unsuccessful");
    }
  }
  // create consumer threads
  // number of consumers
  int num_consumers = check_arg(argv[4]);
  pthread_t consumerid[num_consumers];
  int con_id[num_consumers];
  for(int n=0; n<num_consumers; n++){
    con_id[n]=n+1;
    if((pthread_create(&consumerid[n], NULL, consumer, (void*)&con_id[n])==0)){
      // successful creation of thread
     // cout<<"Successfully created consumer thread"<<endl;
    }
    else{
      // unsuccessful creation of thread
      perror("Consumer thread creation unsuccessful");
    }
  }

  // join threads
  for (int n=0; n<num_producers; n++){
    if((pthread_join(producerid[n], NULL))==0){
      // successful join
    }
    else{
      // unsuccessful  join
      perror("Producer thread join unsuccessful");
    }
  }

  for (int n=0; n<num_consumers; n++){
    if((pthread_join(consumerid[n], NULL))==0){
      // successful join
    }
    else{
      // unsuccessful  join
      perror("Consumer thread join unsuccessful");
    }
  }

  sem_close(semID);

  // TODO
  /*
  int parameter = 10;

  pthread_create (&producerid, NULL, producer, (void *) &parameter);
  
  cout<< "Threads created"<<endl;

  pthread_join (producerid, NULL);

  cout << "Doing some work after the join" << endl;
*/
  return 0;


  sem_timedwait()
}

void *producer (void *pnumber) 
{
  //cout<<"prod number: "<<pnumber<<endl;
  //int prod_n = *((int*)pnumber);
  //cout<<"another prod num:"<<prod_n<<endl;
  //cout<<"n of jobs:"<<n_of_jobs<<endl;
  int job_n=0;
  while(job_n<n_of_jobs){
    // Produce a job with duration between 1-10 seconds
    int job_duration = rand()%10 +1;
    // wait if queue not empty
    sem_wait(semID, EMPTY); 
    /****** CRITICAL SECTION ******/
    sem_wait(semID, MUTEX);
    // Add the job to the circular queue
    //cout<<"End prod num:"<<prod_n<<endl;
    //cout<<"End:"<<circQ.get_end()<<endl;
    //cout<<"Start:"<<circQ.get_start()<<endl;
    circQ.addElement(job_duration);
    printf("Producer(%d): Job id %d duration %d\n", *((int*)pnumber), circQ.get_end(), circQ.get_element(circQ.get_end()));
    sem_signal(semID, MUTEX);
    sem_signal(semID, FULL);

    // wait 1-5 seconds before adding another job
    sleep(rand()%5 +1);
    job_n++;
  }
  

  //int *param = (int *) parameter;

  //cout << "Parameter = " << *param << endl;

  //sleep (5);

  //cout << "\nThat was a good sleep - thank you \n" << endl;

 pthread_exit((void*) 0);
}

void *consumer (void *cnumber) 
{
  int n=0;
  while(n<n_of_jobs){
    // wait if queue is full
    sem_wait(semID, FULL);
    sem_wait(semID, MUTEX);

    // consume item
    int job_id = circQ.get_start();
    int job_consumed = circQ.deleteElement();
    printf("Consumer (%d): Job id %d executing sleep duration %d\n", *((int*)cnumber), job_id, job_consumed);
    sem_signal(semID, MUTEX);

    // sleep for time it takes to process the job
    sleep(job_consumed);

    sem_signal(semID, EMPTY);
    n++;
  }
  

  pthread_exit((void*) 0);

}