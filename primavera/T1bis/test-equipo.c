#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <nSystem.h>

#include "equipo.h"

void registrar_equipo(char **equipo, int crc);

int jugador(char *nom, char ***pequipo) {
  char **equipo= hay_equipo(nom);
  int i= 5;
  int crc= 0;

  for (int k= 0; k<5; k++) {
    char *s= equipo[k];
    while (*s)
      crc += *s++;
    if (strcmp(equipo[k], nom)==0)
      i= k;
  }

  if (i==5)
    nFatalError("jugador", "%s no esta en el equipo\n", nom);

  registrar_equipo(equipo, crc);

  if (pequipo!=NULL)
    *pequipo= equipo;

  return 0;
}

#define N 10000
// Cuidado: M debe ser multiplo de 5
#define M 1000

static char **noms;
static int prox;
static nMonitor m, tabm;
static int tot_equipos= 0, tot_jugadores= 0;

typedef struct {
  char **equipo;
  int crc;
} EquipoCrc;

static EquipoCrc tabla_equipos[N*M];

void registrar_equipo(char **equipo, int crc) {
  int hash= ((intptr_t)equipo / (5*sizeof(char*)))%(N*M);
  EquipoCrc *eqcrc= &tabla_equipos[hash];
  int vuelta= 0;
  nEnter(tabm);
  tot_jugadores++;
  while (eqcrc->equipo!=NULL && eqcrc->equipo!=equipo) {
    hash++;
    eqcrc++;
    if (hash==N*M) {
      if (vuelta)
        nFatalError("registrar_equipo", "Demasiados equipos\n");
      vuelta= 1;
      hash= 0;
      eqcrc= &tabla_equipos[0];
    }
  }
  if (eqcrc->equipo==NULL) {
    eqcrc->equipo= equipo;
    eqcrc->crc= crc;
    tot_equipos++;
    // nPrintf(".");
  }
  else if (eqcrc->crc!=crc) {
    nFprintf(2, "El equipo muto despues del retorno de hay_equipo.\n");
    nFprintf(2, "Esto significa que hay_equipo retorno con el equipo\n");
    nFprintf(2, "incompleto, es decir antes de la llegada de todos los\n");
    nFprintf(2, "jugadores.  Esto es incorrecto.\n");
    nFatalError("registrar_equipo", "Revise su tarea\n");
  }
  nExit(tabm);
}

int mult_jug() {

  nEnter(m);
  while (prox<N*M) {
    char *nom= noms[prox++];
    nExit(m);
    jugador(nom, NULL);
    nEnter(m);
  }
  nExit(m);
  return 0;
}

int nMain() {
  int njug= N*M;
  for (int j=0; j<njug; j++) {
    tabla_equipos[j].equipo= NULL;
  }
  m= nMakeMonitor();
  tabm= nMakeMonitor();
  init_equipo();
  {
    nPrintf("Test 1: el del enunciado\n");
    nTask jugadores[10];
    char *noms[]= {"pedro", "juan", "alberto", "enrique", "diego",
                    "jaime", "jorge", "andres", "jose", "luis"};
    char **equipos[10];
    for (int i=0; i<10; i++) {
      nSleep(100);
      nPrintf("lanzando %s\n", noms[i]);
      jugadores[i]= nEmitTask(jugador, noms[i], &equipos[i]);
    }
    for (int i=0; i<10; i++) {
      nWaitTask(jugadores[i]);
      nPrintf("terminado %s\n", noms[i]);
    }
    for (int i= 1; i<5; i++) {
      if (equipos[i]!=equipos[0])
        nFatalError("main", "%s no esta en el mismo equipo que %s\n",
                noms[i], noms[0]);
    }
    for (int i= 6; i<10; i++) {
      if (equipos[i]!=equipos[5])
        nFatalError("nMain", "%s no esta en el mismo equipo que %s\n",
                noms[i], noms[5]);
    }
    if (equipos[0]==equipos[5])
      nFatalError("nMain", "No pueden ser los mismos equipos\n");
 
    // nFree(equipos[0]);
    // nFree(equipos[5]);
    nPrintf("Test 1: aprobado\n");
  }

  nSetTimeSlice(1);

  {
    nPrintf("Test 2: busca dataraces.\n");
    nPrintf("Se demoro 40 segundos en mi computador con mi solucion.\n");
    nPrintf("Si termina en segmentation fault podria significar que hubo un\n");
    nPrintf("datarace en el que hay_equipo retorno un equipo incompleto y\n");
    nPrintf("el test intento leer el nombre de un jugador que resulto ser\n");
    nPrintf("una direccion invalida.\n");
    noms= nMalloc(N*M*sizeof(char*));
    nTask *jugadores= nMalloc(M*sizeof(nTask));
    for (int i= 0; i<N*M; i++) {
      char *nom= nMalloc(10);
      sprintf(nom, "j%08d", i);
      noms[i]= nom;
    }
    for (int i= 0; i<M; i++) {
      jugadores[i]= nEmitTask(mult_jug);
    }
    nPrintf("\nEsperando\n");
    for (int i= 0; i<M; i++) {
      nWaitTask(jugadores[i]);
    }
    nPrintf("Test 2: aprobado\n");
  }

  nPrintf("Total equipos = %d\n", tot_equipos);
  nPrintf("Total jugadores = %d\n", tot_jugadores);
    
  nPrintf("Felicitaciones, su tarea paso todos los tests\n");
  return 0;
}
