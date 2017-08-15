//static linking, moreover there will be a dynamic linking during compiling phase (-lpthread)
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
//global stuff
int **matrice;
pthread_t tid[3];
struct tipo {
 int val;
 pthread_t id;
};
struct tipo *stack;
int stack_iterator;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t var_cond = PTHREAD_COND_INITIALIZER;
sem_t *sem_con_nome;
int n;
//prototypes
void *produttore(void *args);
void *consumatore(void *args);
//text segment
int main(int args, char *argv[]) {
 //inizialized/not inizialized datas
 int i,j;
 pthread_t ltid;
 n = atoi(argv[1]);
 sem_con_nome = sem_open("sem_con_nome",O_CREAT,0644,1);
 if(sem_con_nome == (void *)-1 ) printf("An error occured while creating semaphore\n");
 //dynamic allocation
 matrice = malloc(sizeof(int*)*n);
 stack = malloc(sizeof(struct tipo)*n*n);
 printf("Matrix:\n");
 for(i=0;i<n;i++) {
  matrice[i] = malloc(sizeof(int)*n);
  for(j=0;j<n;j++) {
   matrice[i][j] = rand()%10;
   printf("%d ",matrice[i][j]);
  }
  printf("\n");
 }
 pthread_create(&ltid,NULL,consumatore,NULL);
 for(i=0;i<3;i++) {
  pthread_create(&tid[i],NULL,produttore,NULL);
 }
 for(i=0;i<3;i++) {
  pthread_join(tid[i],NULL);
 } 
 pthread_join(ltid,NULL);
 return 0;
}
//3 producer threads
void *produttore(void *args) {
 int i,j;
 struct timespec req;
 req.tv_nsec=5000;
 for( ; ; ) {
  i = rand()%n;
  j = rand()%n;
  //critical section
  nanosleep(&req, NULL); //necessary for concurrency
  sem_wait(sem_con_nome);
  if(stack_iterator == n*n) {
   pthread_cond_signal(&var_cond);
   sem_post(sem_con_nome);
   break;
  }
  else {
   stack_iterator++;
   stack[stack_iterator].val = matrice[i][j];
   stack[stack_iterator].id = pthread_self();
  }
  sem_post(sem_con_nome);
   
 }
 sem_destroy(sem_con_nome);
 return 0;
}
//consumer thread
void *consumatore(void *args) {
 int i,a,b,c;
 a = 0;
 b = 0;
 c = 0;
 int best_producer;rtf
 //condition block 
 pthread_mutex_lock(&mutex);
 while(stack_iterator < n*n) 
  pthread_cond_wait(&var_cond,&mutex);
 pthread_mutex_unlock(&mutex);

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
 return 0;
}
