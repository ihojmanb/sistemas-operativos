#include <stdlib.h>
#include <math.h>
#include <nSystem.h>

/* Esta parte es dada */

typedef unsigned long long ulonglong;
typedef unsigned int uint;


int busqueda(ulonglong x, int p, uint inicio, uint final, uint scope_busqueda, int* res){
  // si no nos quedan tareas, terminamos
  if(p == 0){
    return 0;
  }
  /*si nos queda la ultima tarea, buscamos en los numeros restantes
  (que en cantidad pueden ser menos o mas que el scope de bu)*/
  else if(p ==1){
    for (uint k= inicio; k<=final; k++) {
        if (x % k == 0){
           *res = k;
        }
        else{
        }
    }
  }
  else{ // si p > 1, calculo la primera tarea y llamo recursivamente a busqueda.
    for (uint k= inicio; k<=inicio + scope_busqueda - 1; k++) {
      if (x % k == 0){
         *res = k;
      }
      else{
      }
    }
    uint init = inicio + scope_busqueda;
    nTask t = nEmitTask(busqueda, x, p-1, init, final, scope_busqueda, res);
    nWaitTask(t);

  }
return 0; /*para que no alegue el compilador*/
}


uint buscarFactor(ulonglong x, int p, uint inicio, uint fin) {
  int primo = 0; // 0 si es primo, o k si tiene un factor k que lo hace compuesto
  int numeros_disponibles = fin - inicio + 1;
  int scope_busqueda = round((double)numeros_disponibles/p);
  busqueda(x, p, inicio, fin,scope_busqueda, &primo);
  return primo;
}
/* Esta parte es dada */
int nMain(int argc, char **argv) {
  nSetTimeSlice(1);
  ulonglong x= atoll(argv[1]);
  uint p = atoll(argv[2]);
  nPrintf("numero de tareas = %lld \n", p);
  uint raiz_x= (uint)sqrt((double)x);
  nPrintf("x=%lld raiz entera=%u\n", x, raiz_x);
  uint f= buscarFactor(x, p, 2, raiz_x);
  if (f==0)
    nPrintf("primo\n");
  else
    nPrintf("no es primo: factor=%u\n", f);
  return 0;
}
