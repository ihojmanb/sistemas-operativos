int cont =  0; // cantidad de tareas
Info *info;

nSem cola = nMakeSem(0);
nSem mutex = nMakeSem(1);

void difundir(Info *infoP){
  nWaitSem(mutex);
  int tareas_bloqueadas = cont; // num. tareas q esperan
  cont =0;
  nSignalSem(mutex);

  info = infoP;
  while(tareas_bloqueadas--)
    nSignalSem(cola);
}

Info *esperar(){
  // espero el mutex para aumentar la cant. de tareas
  nWaitSem(mutex);
  cont++;
  nSignalSem(mutex); // libero el mutex
  nWaitSem(cola);  /* espero un ticket para liberar
  la info*/
  return info
}
