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
struct type {
 int val;
 pthread_t id;
};
struct type *stack;
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
 matrix = malloc(sizeof(int*)*n);
 if(matrix == (void *)-1 ) {
  printf("An error occured while allocating matrix\n");
  exit(0);
 }
 stack = malloc(sizeof(struct type)*n*n);
 if(stack == (void *)-1 ) {
  printf("An error occured while allocating stack\n");
  exit(0);
 }
 printf("Matrix:\n");
 for(i=0;i<n;i++) {
  matrix[i] = malloc(sizeof(int)*n);
  if(matrix[i] == (void *)-1 ) {
   printf("An error occured while allocating matrix\n");
   exit(0);
  }
  for(j=0;j<n;j++) {
   matrix[i][j] = rand()%10;
   printf("%d ",matrix[i][j]);
  }
  printf("\n");
 }
 if(pthread_create(&ltid,NULL,consumer,NULL)==-1) {
  printf("An error occured while creating thread\n");
  exit(0);
 }
 for(i=0;i<3;i++) {
  if(pthread_create(&tid[i],NULL,producer,NULL)==-1) {
   printf("An error occured while creating thread\n");
   exit(0);
  }
 }
 for(i=0;i<3;i++) {
  pthread_join(tid[i],NULL);
 } 
 pthread_join(ltid,NULL);
 return 0;
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
   stack[stack_iterator].val = matrix[i][j];
   stack[stack_iterator].id = pthread_self();
  }
  sem_post(sem_with_name);
   
 }

 if(sem_destroy(sem_with_name)==-1) {
  printf("An error occured while destroying semaphore\n");
 }
 exit(0)
}

//consumer thread
void *consumer(void *args) {
 int i,a,b,c;
 a = 0;;
 b = 0;
 c = 0;
 int best_producer;

 //condition block 
 pthread_mutex_lock(&mutex);
 if(stack_iterator < n*n) 
  pthread_cond_wait(&var_cond,&mutex);
 pthread_mutex_unlock(&mutex);

 //find the best producer
 for(i=0;i<n*n;i++) {
  printf("thread: %d value: %d\n",(int)stack[i].id, stack[i].val);
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
 exit(0);
}
