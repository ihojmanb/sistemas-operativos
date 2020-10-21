#include <nSystem.h>

#include "equipo.h"

char **equipo; // = nMalloc(5*sizeof(char*)); equipo en formación
int k= 0;      // cuantos jugadores han llegado
nSem m;        // = nMakeSem(1); para garantizar la exclusión mutua
nSem **esperando; // = arreglo semáforos para jugadores que esperan

void init_equipo(void) {
  equipo= nMalloc(5*sizeof(char*));
  m= nMakeSem(1);
  esperando= nMalloc(4*sizeof(nSem));
}

char **hay_equipo(char *nom) {
  nWaitSem(m); // inicio secc. crítica
  equipo[k++]= nom;
  char **miequipo= equipo;
  nSem listo = nMakeSem(0); // 0 ticket cuando el equipo no esta listo, 1 ticket
  //cuando lo está
  if (k!=5) { // aún no hay equipo
    esperando[k-1] = listo;
    nSignalSem(m); // fin secc. crít.
    //listo no comparte memoria con otros threads, podemos llamarlo sin la exclusion mutua
    nWaitSem(listo); // espera a que el 5to jugador avise que el equipo esta completo
    nDestroySem(listo);
  }
  else { // hay equipo: despertar a los 4 jugadores en espera
    for (int i= 0; i<4; i++)
      nSignalSem(esperando[i]);
    // preparar nuevo equipo
    equipo= nMalloc(5*sizeof(char*));
    k= 0;
    nDestroySem(listo);
    nSignalSem(m); // fin secc. crít.
  }
  return miequipo;
}
