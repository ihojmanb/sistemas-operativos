// especificar la estructura de datos Orden:
typedef struct{
  Pizza *pizza;
  int cocida; //indica si esta lista para ser entregada
} Orden;

Horno* horno;
//declarar el monitor
nMonitor ctrl;
FiFoQueue *pendientes; // ordenes pendientes

nTask pizzeriaTask; // thread adicional para operar el horno


Pizza *obtenerPizzaCruda();

void hornear(Horno* horno, Pizza* pizza_vec, int n_pizzas);

// implementar eficientemente las siguientes funciones:

void iniciarPizzeria(){
  ctrl = nMakeMonitor();
  pendientes = makeFifoQueue();
  pizzeriaTask = nEmitTask(horno);
}

int horno(){ //manejo del horno
  for(;;){
    Pizza *pizza_vec[4];
    Orden *orden_vec[4];
    nEnter(ctrl);
    while(EmptyFifoQueue(pendientes))
        nWait(ctrl)
    int k = 0;
    while (!EmptyFifoQueue(pendientes) && k < 4) {
      orden_vec[k] = (Orden*) GetObj(pendientes);
      pizza_vec[k] = orden_vec[k]->pizza;
      k++;
    }
    nExit(ctrl);
    hornear(horno. pizza_vec, k);

    nEnter(ctrl);
    int i;
    for(i=0; i < k; i++){
      orden_vec[i]->cocida = true;
    }
    nNotifyAll(ctrl);
    nExit(ctrl);
  }

}

// estas funciones son llamadas con los programadores
Orden* ordenarPizza(){
  Orden *po = (Orden*)nMalloc(sizeof(Orden));
  po->pizza = obtenerPizzaCruda();
  po->cocida = FALSE;
  nEnter(ctrl);
  PutObj(pendientes, po); //encolando la orden
  nNotifyAll(ctrl);
  nExit(ctrl);
  return po;

}
Pizza* esperarPizza(Orden* orden){
  nEnter(ctrl);
  while(!orden->cocida){
    nWait(ctrl);
  }
  nExit(ctrl);
  return orden->pizza;
}

Pizza* comprarPizza(){
  Orden *o = ordenarPizza();
  Pizza *p = esperarPizza(o);
  return p;
}
