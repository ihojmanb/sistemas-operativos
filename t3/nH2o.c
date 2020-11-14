#include "nSystem.h"
#include "nSysimp.h"
#include "fifoqueues.h"

//... declare variables globales ...
FifoQueue oQueue; // invocaciones pendientes de nCombineOxy
FifoQueue hQueue; // invocaciones pendientes de nCombineHydro


void H2oInit() {
  //... función de inicializacion ...
  //... invoque esta funcion desde nMain.c ...
  oQueue = MakeFifoQueue();
  hQueue = MakeFifoQueue();

}


// Esta funcion es invocada desde testh2o.c
nH2o nCombineOxy(void *oxy, int timeout) {
  START_CRITICAL();
  nTask this_task = current_task;
  this_task->oxy = oxy;
  this_task->molecula_ready = FALSE;

  // si no hay hidrogeno, espera
  if(LengthFifoQueue(hQueue) < 2){
    PutObj(oQueue, this_task);
    this_task->status = WAIT_COMBINE;
    ResumeNextReadyTask();
  }

  // si hay 2H, este O despierta a las tareas
  else if (!this_task->molecula_ready){
      nTask h1 = GetObj(hQueue);
      nTask h2 = GetObj(hQueue);
      h1->molecula_ready = TRUE;
      h2->molecula_ready = TRUE;

      nH2o h2o = nMalloc(sizeof(*h2o));

      h2o->oxy = oxy;
      h2o->hydro1 = h1->hydro;
      h2o->hydro2 = h2->hydro;
      h1->molecula =  h2o;
      h2->molecula =  h2o;
      this_task->molecula = h2o;

      h1->status = READY;
      h2->status = READY;
      PushTask(ready_queue, h1);
      PushTask(ready_queue, h2);

  }
  END_CRITICAL();
  return this_task->molecula;
}

// Esta funcion es invocada desde testh2o.c
nH2o nCombineHydro(void *hydro) {
  START_CRITICAL();
  nTask this_task = current_task;
  this_task->hydro = hydro;
  this_task->molecula_ready = FALSE;
  PutObj(hQueue, this_task);
  if(LengthFifoQueue(oQueue) < 1 || LengthFifoQueue(hQueue) < 2){
    this_task->status = WAIT_COMBINE;
    ResumeNextReadyTask();
  }

  // el proceso puso el ultimo átomo necesario
  // si la molecula no está lista, la armamos
  else if (!this_task->molecula_ready){
      nTask h1 = GetObj(hQueue);
      nTask h2 = GetObj(hQueue);
      nTask o = GetObj(oQueue);
      h1->molecula_ready = TRUE;
      h2->molecula_ready = TRUE;
      o->molecula_ready = TRUE;

      nH2o h2o= nMalloc(sizeof(*h2o));

      h2o->oxy = o->oxy;
      h2o->hydro1 = h1->hydro;
      h2o->hydro2 = h2->hydro;
      h1->molecula =  h2o;
      h2->molecula =  h2o;
      o->molecula =  h2o;

      h1->status = READY;
      o->status = READY;
      PushTask(ready_queue, h1);
      PushTask(ready_queue, o);
  }
  END_CRITICAL();
  return current_task->molecula;
}
