/*
best_producer: 
The program creates 3 producer threads and one consumer; moreover, it provides a dynamic
allocated bidimensional vector of randomized int (dimension of matrix : n, passed as 
argument); finally, a stack with the following record type: id of a thread, an integer
value (dimension of stack : n*n) . The main thread start the other ones: producers go on
taking elements at random from the matrix and putting them on the stack (this was 
implemented guaranteeing mutual exclusion of writing for every thread); meanwhile the
consumer wait on a condition variable (swapping out from RAM) and is signaled when the stack 
is fullfilled, then it prints all elements in the stack and the tid of the thread that put 
the highest number of values in the stack; finally every thread terminates. For synchronization 
two techniques were used: a named POSIX semaphore, initialized with 1 resource for producers
and a condition variable (with an associated mutex) for consumer (in order to avoid busy waiting).
*/
//static linking, moreover there will be a dynamic linking during compiling phase (-lpthread)
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

//global stuff
int **matrix;
pthread_t tid[3];
struct stack_record {
 int value;
 pthread_t id;
};
struct stack_record *stack;
int stack_iterator;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t var_cond = PTHREAD_COND_INITIALIZER;
sem_t *sem_with_name;
int n;

//prototypes
void *producer(void *args);
void *consumer(void *args);

//text segment
int main(int args, char *argv[]) {

 //inizialized/not inizialized datas
 int i,j;
 pthread_t ltid;
 n = atoi(argv[1]);
 sem_with_name = sem_open("sem_with_name",O_CREAT,0644,1);
 if(sem_with_name == (void *)-1 ) {
  printf("An error occured while creating semaphore\n");
  exit(0);
 }
 
 //dynamic allocation
 if((matrix = malloc(sizeof(int*)*n)) == (void *)-1) {
  printf("An error occured while allocating matrix\n");
  exit(0);
 }
 if((stack = malloc(sizeof(struct stack_record)*n*n)) == (void *)-1) {
  printf("An error occured while allocating stack\n");
  exit(0);
 }
 printf("Matrix:\n");
 for(i=0;i<n;i++) {
  if((matrix[i] = malloc(sizeof(int)*n)) == (void *)-1) {
   printf("An error occured while allocating matrix\n");
   exit(0);
  }
  for(j=0;j<n;j++) {
   matrix[i][j] = rand()%10;
   printf("%d ",matrix[i][j]);
  }
  printf("\n");
 }
 pthread_create(&ltid,NULL,consumer,NULL);
 for(i=0;i<3;i++) {
  pthread_create(&tid[i],NULL,producer,NULL);
 }
 for(i=0;i<3;i++) {
  pthread_join(tid[i],NULL);
 } 
 pthread_join(ltid,NULL);
 exit(0);
}

//3 producer threads
void *producer(void *args) {
 int i,j;
 struct timespec req;
 req.tv_nsec=5000;
 for( ; ; ) {
  i = rand()%n;
  j = rand()%n;
  
  //critical section
  nanosleep(&req, NULL); //necessary for concurrency
  sem_wait(sem_with_name);
  if(stack_iterator == n*n) {
   pthread_cond_signal(&var_cond);
   sem_post(sem_with_name);
   break;
  }
  else {
   stack_iterator++;
   stack[stack_iterator].value = matrix[i][j];
   stack[stack_iterator].id = pthread_self();
  }
  sem_post(sem_with_name);
   
 }
 sem_destroy(sem_with_name);
 pthread_exit(NULL);
}

//consumer thread
void *consumer(void *args) {
 int i,a,b,c;
 a = 0;
 b = 0;
 c = 0;
 int best_producer;
 
 //condition block 
 pthread_mutex_lock(&mutex);
 if(stack_iterator < n*n) 
  pthread_cond_wait(&var_cond,&mutex);
 pthread_mutex_unlock(&mutex);

 //best producer linear research (O(stack_size))
 for(i=0;i<n*n;i++) {
  printf("thread: %d value: %d\n",(int)stack[i].id, stack[i].value);
  if(pthread_equal(stack[i].id,tid[0])) a++;
  if(pthread_equal(stack[i].id,tid[1])) b++;
  if(pthread_equal(stack[i].id,tid[2])) c++;
 }
 if(a > b) {
   best_producer = (int)tid[0];
   if(c > a) best_producer = (int)tid[2];
 }
 else {
   best_producer = (int)tid[1];
   if(c > b) best_producer = (int)tid[2]; 
 }
 printf("best_producer: %d", best_producer);
 pthread_exit(NULL);
}
