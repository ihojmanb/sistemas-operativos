#include <nSystem.h>
#include <fifoqueues.h>
#include "batch.h"

/*OK*/
struct job{
  JobFun funcion;
  void *args;
  void*res;
  int finalizado;
};

int procesar();
nMonitor m;
FifoQueue q;
int jobs_creados;
int jobs_completados;
int on;
int num;
nTask *threads;
/*CONDICIONES*/
nCondition wait_stop; //espera a que se apague el sistema
nCondition wait_job;//espera a que se ejecute el job
nCondition wait_queue; //espera elementos en la cola

/*OK*/
void startBatch(int n) {
  m = nMakeMonitor();
  q = MakeFifoQueue();
  wait_stop = nMakeCondition(m);
  wait_job = nMakeCondition(m);
  wait_queue = nMakeCondition(m);
  jobs_creados = 0;
  jobs_completados = 0;
  num = n;
  on = 1;
  threads = nMalloc(n*sizeof(nTask));
  for (int k = 0; k < n; k++) {
    threads[k] = nEmitTask(procesar);
  }
}

/*OK*/
Job *submitJob(JobFun fun, void *input) {
  Job *p_new_job = (Job*)nMalloc(sizeof(Job));
  p_new_job->funcion= fun;
  p_new_job->args = input;
  p_new_job->finalizado = 0;
  nEnter(m);
  PutObj(q, p_new_job); //encolando el job
  jobs_creados += 1;
  nSignalCondition(wait_queue);
  nExit(m);
  return p_new_job;
}


/*OK*/
void *waitJob(Job *job) {
  nEnter(m);
  while (!job->finalizado) {
    nWaitCondition(wait_job);
  }
  jobs_completados += 1;
  if(jobs_completados == jobs_creados){
    nSignalCondition(wait_stop);
  }
  nExit(m);
  // nPrintf("job completado\n");
  return job->res;
}

int procesar(){
  Job *pjob;
  while(on){
    /*tomar un job*/
    nEnter(m);
    /*mientras el sistema este on y la cola esté vacía*/
    if(EmptyFifoQueue(q)){
      nWaitCondition(wait_queue);
    }

    pjob = (Job*)GetObj(q);
    /*si la cola esta vacia*/
    nExit(m);
    if(pjob==NULL){
      return 0;
    }
    /*si la cola no esta vacia*/
    /*ejecutar su funcion*/
    pjob->res = (pjob->funcion)(pjob->args);
    /*notificar que se ejecutó la funcion*/
    nEnter(m);
    pjob->finalizado = 1;
    nSignalCondition(wait_job);
    nExit(m);
  }
  return 0;
}
void stopBatch() {

  nEnter(m);
  while (jobs_creados != jobs_completados) {
    nWaitCondition(wait_stop);
  }
  on=0;
  /*despertamos a todos los threads que quedan esperando*/
  for(int i = 0; i < num; i++){
    nSignalCondition(wait_queue);
  }
  nExit(m);
  for(int i = 0; i < num; i++){
    nWaitTask(threads[i]); // aqui se queda pegado
  }

}
