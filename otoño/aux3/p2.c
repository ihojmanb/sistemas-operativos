#define P 8;
Camion *camiones[P]; //flota de
Ciudad *ubic [P]; // arreglo de ubicaciones
int *ocupados[P]; //indica el estado del camion, si esta ocioso u ocupado

nMonitor m; // nMakeMonitor()

double distancia(Ciudad *o, Ciudad*d);


void transportar(Contenedor *cont, Ciudad *orig, Ciudad *dest){

  //elegir el camion a usar
  int k;
  int mejor = -1; // indice del camion más cercano
  nEnter(m);
  for(;;){
    for (k = 0; k < P; unt i++) {
      if(!ocupados[k] && (mejor < 0 || distancia(ubic[k], orig) < distancia(ubic[mejor], orig))){
        mejor = k;
      }
    }
    if (k == -1){
      nWait(m);
    }
    else{
      break;
    }
  }
  ocupados[mejor] = TRUE;
  nExit(m);
  conducir(camiones[mejor], ubic, orig);
  cargar(camiones[mejor], cont);
  conducir(camiones[mejor], orig, dest);
  descargar(camiones[mejor], cont);

  nEnter(m);
  ocupados[mejor] = FALSE;
  ubic[mejor] = dest;
  nNotifyAll(m);
  nExit(m);











}
