#define _DEFAULT_SOURCE 1

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#include "nSystem.h"
#include "fifoqueues.h"

#define NARGS 14

// El descriptor de tarea
struct ptask {
  pthread_t pthr; // Identificador del pthread
  int rc;         // Codigo de retorno usado en nExit y nReply
  char *taskname; // Para fines de debugging
  long args[NARGS]; // Los parametros recibidos en nEmitTask
  int (*proc)();  // La funcion raiz que ejecuta esta tarea

  // Para la recepcion de mensajes
  pthread_mutex_t msgmutex;
  pthread_cond_t msgcond;
  struct node *msgqueue;

  // Para la respuesta de mensajes
  pthread_mutex_t replymutex;
  pthread_cond_t replycond;
  int replied;
};

// La variable current_thread es local a cada thread (atributo __thread)
// y almacena la direccion del descriptor de tarea
static __thread nTask current_task;

// Funciones requeridas por la implementacion de psystem

static long long getTimeNanos();
static void resetTime();

/*************************************************************
 * La tarea principal provista por el programador
 *************************************************************/

static nTask makeTask(const char *const_taskname) {
  nTask task= malloc(sizeof(struct ptask));
  if (const_taskname==NULL)
    task->taskname= NULL;
  else {
    char *taskname= malloc(strlen(const_taskname)+1);
    strcpy(taskname, const_taskname);
    task->taskname= taskname;
  }
  pthread_mutex_init(&task->msgmutex, NULL);
  pthread_cond_init(&task->msgcond, NULL);
  pthread_mutex_init(&task->replymutex, NULL);
  pthread_cond_init(&task->replycond, NULL);
  task->msgqueue= NULL;
  return task;
}

static void destroyTask(nTask task) {
  if (task->taskname!=NULL)
    free(task->taskname);
  pthread_mutex_destroy(&task->msgmutex);
  pthread_cond_destroy(&task->msgcond);
  pthread_mutex_destroy(&task->replymutex);
  pthread_cond_destroy(&task->replycond);
  free(task);
}

int main(int argc, char *argv[]) {
  resetTime();

  // Descriptor de la tarea principal
  nTask main_task= makeTask("nMain");
  current_task= main_task;

  // nMain es la funcion principal en nSystem
  int rc= nMain(argc, argv);

  destroyTask(main_task);

  return rc;
}

/*************************************************************
 * Los servicios
 *************************************************************/

#ifndef BUFFSIZE
#define BUFFSIZE 800
#endif

static int Gprintf(int fd, char *format, va_list ap) {
  char buf[BUFFSIZE];

  int len= vsnprintf(buf, BUFFSIZE, format, ap);

  if (len>=BUFFSIZE)
    nFatalError("nFprintf", "Se excede el taman~o del buffer\n");

  int written=0;

  while (written<len) {
    int rc= write(fd, &buf[written], len-written);
    if (rc<0)
      nFatalError("nFprintf", "write return error\n");
    written += rc;
  }

  return len;
}

int nPrintf( char *format, ... ) {
  int rc;
  va_list ap;

  va_start(ap, format);
  // rc= vprintf(format, ap);
  rc= Gprintf(1, format, ap);
  va_end(ap);

  // fflush(stdout);

  return rc;
}

int nFprintf( int fd, char *format, ... ) {
  int rc;
  va_list ap;

  va_start(ap, format);
  rc= Gprintf(fd, format, ap);
  va_end(ap);

  return rc;
}

static int lock= 0;

void nFatalError( char *procname, char *format, ... ) {
  va_list ap;

  // Tratamos de evitar llamadas recursivas 
  if (lock==1) return; // Pase lo que pase 8^)
  lock=1;

  va_start(ap, format);
  nFprintf(2,"Error Fatal en la rutina %s y la tarea \n", procname);
  nFprintf(2, "%s\n", current_task->taskname);
  Gprintf(2, format, ap);
  va_end(ap);

  nExitSystem(1); /* shutdown */
}

void *nMalloc(size_t size) {
  return malloc(size);
}

void nFree(void *ptr) {
  free(ptr);
}

/*************************************************************
 * Creacion y manejo de tareas
 *************************************************************/

static void *task_fun(void *ptr) {
  nTask task= ptr;
  int (*proc)()= task->proc;
  current_task= task;
  long *a= task->args;
  // soporta hasta 14 argumentos enteros (o 7 punteros de 64 bits)
  int rc= (*proc)(a[0], a[1], a[2], a[3], a[4], a[5], a[6],
                  a[7], a[8], a[9], a[10], a[11], a[12], a[13]);
  nExitTask(rc);
  return NULL;
}

nTask nEmitTask( int (*proc)(), ... ) {
  nTask task= makeTask(NULL);
  task->proc= proc;
  va_list ap;
  va_start(ap, proc);
  for (int i=0; i<NARGS; i++)
    task->args[i]= va_arg(ap, long);
  va_end(ap);
  pthread_create(&task->pthr, NULL, task_fun, task);
  return task;
}

void nExitTask(int rc) {
  current_task->rc= rc;
  pthread_exit(NULL);
}

int nWaitTask(nTask task) {
  pthread_join(task->pthr, NULL);
  int rc= task->rc;
  destroyTask(task);
  return rc;
}

void nExitSystem(int rc) {
  exit(rc);
}

long long Real2LL(double rv) {

  long long *pll= (long long*)&rv;
  return *pll;
}

double LL2Real(long long llv) {
  double *prv= (double*)&llv;
  return *prv;
}

/*************************************************************
 * Definicion de parametros para las tareas
 *************************************************************/

int nSetStackSize(int size) { // No hacer nada y suponer que es de 8 MB
  return 8*1024*1024;
}

void nSetTimeSlice(int slice) { // Ignorar
}

#define MAXNAMESIZE 200

void nSetTaskName(char *format, ...) {
  char taskname[MAXNAMESIZE];
  va_list ap;
  va_start(ap, format);
  int len= vsnprintf(taskname, MAXNAMESIZE, format, ap);
  va_end(ap);
  if (len==MAXNAMESIZE) {
    taskname[MAXNAMESIZE-1]= 0;
    len--;
  }

  if (current_task->taskname!=NULL)
    free(current_task->taskname);
  current_task->taskname= strcpy(malloc(len+1), taskname);
}

char* nGetTaskName() {
  return current_task->taskname;
}

nTask nCurrentTask() {
  return current_task;
}

int nGetContextSwitches() { // No se puede implementar
  return 0;
}

int nGetQueueLength() { // No se puede implementar
  return 0;
}

/*************************************************************
 * Semaforos
 *************************************************************/

struct psem {
  pthread_mutex_t mtx;
  int count;
  FifoQueue q;
};

typedef struct {
  int rdy;
  pthread_cond_t cond;
} SemReq;

nSem nMakeSem(int count) {
  nSem sem= malloc(sizeof(struct psem));
  pthread_mutex_init(&sem->mtx, NULL);
  sem->count= count;
  sem->q= MakeFifoQueue();
  return sem;
}

void nWaitSem(nSem sem) {
  pthread_mutex_lock(&sem->mtx);
  if (sem->count>0)
    sem->count--;
  else {
    SemReq req= { FALSE, PTHREAD_COND_INITIALIZER };
    PutObj(sem->q, &req);
    while (!req.rdy)
      pthread_cond_wait(&req.cond, &sem->mtx);
    pthread_cond_destroy(&req.cond);
  } 
  pthread_mutex_unlock(&sem->mtx);
}

void nSignalSem(nSem sem) {
  pthread_mutex_lock(&sem->mtx);
  if (EmptyFifoQueue(sem->q))
    sem->count++;
  else {
    SemReq *preq= GetObj(sem->q);
    preq->rdy= TRUE;
    pthread_cond_broadcast(&preq->cond);
  }
 
  pthread_mutex_unlock(&sem->mtx);
}

void nDestroySem(nSem sem) {
  pthread_mutex_destroy(&sem->mtx);
  DestroyFifoQueue(sem->q);
  free(sem);
}

/*************************************************************
 * Monitores
 *************************************************************/

struct pmonitor {
  pthread_mutex_t mtx;
  pthread_cond_t cond;
};

nMonitor nMakeMonitor() {
  nMonitor mon= malloc(sizeof(struct pmonitor));
  pthread_mutex_init(&mon->mtx, NULL);
  pthread_cond_init(&mon->cond, NULL);
  return mon;
}

void nDestroyMonitor(nMonitor mon) {
  pthread_mutex_destroy(&mon->mtx);
  pthread_cond_destroy(&mon->cond);
}

void nEnter(nMonitor mon) {
  pthread_mutex_lock(&mon->mtx);
}

void nExit(nMonitor mon) {
  pthread_mutex_unlock(&mon->mtx);
}

void nWait(nMonitor mon) {
  pthread_cond_wait(&mon->cond, &mon->mtx);
}

void nNotifyAll(nMonitor mon) {
  pthread_cond_broadcast(&mon->cond);
}

struct pcondition {
  pthread_mutex_t *pmtx;
  pthread_cond_t pthr_cond;
};

nCondition nMakeCondition(nMonitor mon) {
  nCondition cond= malloc(sizeof(struct pcondition));
  cond->pmtx= &mon->mtx;
  pthread_cond_init(&cond->pthr_cond, NULL);
  return cond;
}

void nDestroyCondition(nCondition cond) {
  pthread_cond_destroy(&cond->pthr_cond);
  free(cond);
}

void nWaitCondition(nCondition cond) {
  pthread_cond_wait(&cond->pthr_cond, cond->pmtx);
}
  
void nSignalCondition(nCondition cond) {
  pthread_cond_signal(&cond->pthr_cond);
}

/*************************************************************
 * Mensajes
 *************************************************************/

typedef struct node {
  void *msg;
  nTask sender;
  struct node *next;
} Node;

int nSend(nTask task, void *msg) {
  nTask this_task= current_task;

  // Queue message in task->msgqueue
  pthread_mutex_lock(&task->msgmutex);
  this_task->replied= 0;
  Node node= {msg, this_task, task->msgqueue};
  task->msgqueue= &node;
  pthread_cond_signal(&task->msgcond);
  pthread_mutex_unlock(&task->msgmutex);
 
  // Wait reply

  pthread_mutex_lock(&this_task->replymutex);
  while (!this_task->replied)
    pthread_cond_wait(&this_task->replycond, &this_task->replymutex);
  int rc= this_task->rc;
  pthread_mutex_unlock(&this_task->replymutex);

  return rc;
}

void *nReceive(nTask *ptask, int max_delay) {
  nTask this_task= current_task;

  pthread_mutex_lock(&this_task->msgmutex);

  Node *pnode= this_task->msgqueue;
  if (pnode==NULL && max_delay!=0) {
    long long ini_nsec= getTimeNanos();
    long long end_nsec= ini_nsec + max_delay*1000000LL;
    int tv_sec= max_delay/1000;
    struct timespec ts= {tv_sec, max_delay-tv_sec*1000};
    for (;;) {
      if (max_delay<0)
        pthread_cond_wait(&this_task->msgcond, &this_task->msgmutex);
      else
        pthread_cond_timedwait(&this_task->msgcond, &this_task->msgmutex, &ts);
      pnode= this_task->msgqueue;
      if (pnode!=NULL)
        break;
      if (max_delay<0)
        continue;
      long long cur_nsec= getTimeNanos();
      long long rem_nsec= end_nsec-cur_nsec;
      if (rem_nsec<=0)
        break;
      ts.tv_sec= rem_nsec/1000000000LL;
      ts.tv_nsec= rem_nsec - ts.tv_sec*1000000000LL;
    }
  }

  void *msg;

  if (pnode==NULL) {
    msg= NULL;
    if (ptask!=NULL)
      *ptask= NULL;
  }
  else {
    this_task->msgqueue= pnode->next;
    msg= pnode->msg;
    if (ptask!=NULL)
      *ptask= pnode->sender;
  }

  pthread_mutex_unlock(&this_task->msgmutex);
  return msg;
}

void nReply(nTask task, int rc) {
  pthread_mutex_lock(&task->replymutex);
  if (task->replied!=0)
    nFatalError("nReply", "Message was already replied\n");
  task->replied= 1;
  task->rc= rc;
  pthread_cond_signal(&task->replycond);
  pthread_mutex_unlock(&task->replymutex);
}

void nSleep(int delay) {
  if (delay<=0)
    return;

  time_t tv_sec= delay / 1000;
  long milis= delay - tv_sec*1000;
  struct timespec ts = { tv_sec, milis*1000000L };
  nanosleep(&ts, NULL);
}

static long long getTimeNanos() {
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return (long long)ts.tv_sec*1000000000LL+ts.tv_nsec;
}

static long long time0= 0;

static void resetTime() {
  time0= getTimeNanos();
}

int nGetTime() {
  return (getTimeNanos()-time0)/1000000;
}

/*************************************************************
 * E/S basica
 *************************************************************/

/* Estas funciones son equivalentes a open, close, read y write en
 * Unix.  Las ``nano'' funciones son no bloqueantes para el proceso Unix,
 * solo bloquean la tarea que las invoca.
 */

int nOpen( char *path, int flags, ... ) {
  va_list ap;
  va_start(ap, flags);
  int mode= va_arg(ap, int);
  va_end(ap);

  int fd= open(path, flags, mode);

  return fd;
}

int nClose(int fd) {
  return close(fd);
}

int nRead(int fd, char *buf, int nbytes) {
  return read(fd, buf, nbytes);
}

int nWrite(int fd, char *buf, int nbytes) {
  return write(fd, buf, nbytes);
}

void nReopenStdio() {
}

void nSetNonBlockingStdio() {
}
