#include <nSystem.h>
#include "pub.h"

typedef struct {
  int entrada, salida;
} Tiempos;

static int verbose= TRUE;

static int tiempo_actual= 1; /* en centesimas de segundo */

void pausa(int tiempo_espera) { /* tiempo_espera en centesimas de segundo */
  int tiempo_inicio= tiempo_actual;
  nSleep(tiempo_espera*300);
  tiempo_actual= tiempo_inicio+tiempo_espera;
}

static int adentro[2]= {0, 0}; /* adentro[0]: varones dentro del banno */
                               /* adentro[1]: damas dentro del banno */
nSem mutex;                    /* mutex para acceder a adentro */

int humano(char *nombre, int sexo, int tiempo_estadia, Tiempos *pt) {
  if (verbose)
    nPrintf("%d: %s solicita entrar\n", tiempo_actual, nombre);

  entrar(sexo); /* <----- Ud. debe programar esta funcion en pub.c */
  pt->entrada= tiempo_actual;

  nWaitSem(mutex);
  adentro[sexo]++;
  if (adentro[!sexo]>0)
    nFatalError("humano",
                "no se cumple la exclusion mutua de varones y damas\n");
  nSignalSem(mutex);
  if (verbose)
    nPrintf("%d: %s entra\n", tiempo_actual, nombre);
  if (tiempo_estadia!=0)
    pausa(tiempo_estadia);
  if (verbose)
    nPrintf("%d: %s sale\n", tiempo_actual, nombre);
  nWaitSem(mutex);
  adentro[sexo]--;
  nSignalSem(mutex);

  salir(sexo); /* <----- Ud. debe programar esta funcion en pub.c */
  pt->salida= tiempo_actual;

  return 0;
}

void verificar(Tiempos *pt, int entrada, int salida, char *humano) {
  if (pt->entrada!=entrada)
    nFatalError("verificar",
         "Tiempo de entrada incorrecto de %s.  Es %d y debio ser %d.\n",
         humano, pt->entrada, entrada);
  if (pt->salida!=salida)
    nFatalError("verificar",
         "Tiempo de salida incorrecto de %s.  Es %d y debio ser %d.\n",
         humano, pt->salida, salida);
}

int nMain() {
  nPrintf("%d\n", sizeof(char*));
  nTask pedro, juan, diego, paco, maria, ana, silvia;
  Tiempos t_pedro, t_juan, t_diego, t_paco, t_maria, t_ana, t_silvia;
  mutex= nMakeSem(1);

  ini_pub(); /* <----- Ud. debe programar esta funcion en pub.c */

  nPrintf("--- Primer test -------------\n");
  nPrintf("1: pedro solicita entrar, pedro entra\n");
  nPrintf("2: pedro sale\n");
  nPrintf("==>\n");
  tiempo_actual= 1;
  pedro= nEmitTask(humano, "pedro", VARON, 1, &t_pedro);
  nWaitTask(pedro);
  verificar(&t_pedro, 1, 2, "pedro");

  nPrintf("--- 2do. test ---------------\n");
  nPrintf("1: maria solicita entrar, maria entra\n");
  nPrintf("2: maria sale\n");
  nPrintf("==>\n");
  tiempo_actual= 1;
  maria= nEmitTask(humano, "maria", DAMA, 1, &t_maria);
  nWaitTask(maria);
  verificar(&t_maria, 1, 2, "maria");

  nPrintf("--- 3er. test ---------------\n");
  nPrintf("1: pedro solicita entrar, pedro entra\n");
  nPrintf("2: maria solicita entrar\n");
  nPrintf("3: pedro sale, maria entra\n");
  nPrintf("4: maria sale\n");
  nPrintf("==>\n");
  tiempo_actual= 1;
  pedro= nEmitTask(humano, "pedro", VARON, 2, &t_pedro);
  pausa(1);
  maria= nEmitTask(humano, "maria", DAMA, 1, &t_maria);
  nWaitTask(pedro);
  nWaitTask(maria);
  verificar(&t_pedro, 1, 3, "pedro");
  verificar(&t_maria, 3, 4, "maria");


  nPrintf("--- 4to. test --------------------------------------------\n");
  nPrintf("1: maria solicita entrar, maria entra\n");
  nPrintf("2: pedro solicita entrar\n");
  nPrintf("3: maria sale, pedro entra\n");
  nPrintf("4: pedro sale\n");
  nPrintf("==>\n");
  tiempo_actual= 1;
  maria= nEmitTask(humano, "maria", DAMA, 2, &t_maria);
  pausa(1);
  pedro= nEmitTask(humano, "pedro", VARON, 1, &t_pedro);
  nWaitTask(pedro);
  nWaitTask(maria);
  verificar(&t_maria, 1, 3, "maria");
  verificar(&t_pedro, 3, 4, "pedro");


  /* 5to. test: pedro entra, juan entra, maria solicita entrar y espera,
   * pedro sale, juan sale, maria entra y sale.
   */
  
  nPrintf("--- 5to. test --------------------------------------------\n");
  nPrintf("1: pedro solicita entrar, pedro entra\n");
  nPrintf("2: juan solicita entrar, juan entra\n");
  nPrintf("3: maria solicita entrar\n");
  nPrintf("4: pedro sale\n");
  nPrintf("5: juan sale, maria entra\n");
  nPrintf("6: maria sale\n");
  nPrintf("==>\n");
  tiempo_actual= 1;
  pedro= nEmitTask(humano, "pedro", VARON, 3, &t_pedro);
  pausa(1);
  juan= nEmitTask(humano, "juan", VARON, 3, &t_juan);
  pausa(1);
  maria= nEmitTask(humano, "maria", DAMA, 1, &t_maria);
  nWaitTask(pedro);
  nWaitTask(juan);
  nWaitTask(maria);
  verificar(&t_pedro, 1, 4, "pedro");
  verificar(&t_juan, 2, 5, "juan");
  verificar(&t_maria, 5, 6, "maria");

  nPrintf("--- 6to. test --------------------------------------------\n");
  nPrintf("1: maria solicita entrar,maria entra\n");
  nPrintf("2: ana solicita entrar, ana entra\n");
  nPrintf("3: juan solicita entrar\n");
  nPrintf("4: ana sale\n");
  nPrintf("5: maria sale, juan entra\n");
  nPrintf("6: juan sale\n");
  nPrintf("==>\n");
  tiempo_actual= 1;
  maria= nEmitTask(humano, "maria", DAMA, 4, &t_maria);
  pausa(1);
  ana= nEmitTask(humano, "ana", DAMA, 2, &t_ana);
  pausa(1);
  juan= nEmitTask(humano, "juan", VARON, 1, &t_juan);
  nWaitTask(juan);
  nWaitTask(ana);
  nWaitTask(maria);
  verificar(&t_maria, 1, 5, "maria");
  verificar(&t_ana, 2, 4, "ana");
  verificar(&t_juan, 5, 6, "juan");

  nPrintf("--- 7mo. test ------------------------------------------\n");
  nPrintf("1: maria solicita entrar, maria entra\n");
  nPrintf("2: ana solicita entrar, ana entra\n");
  nPrintf("3: juan solicita entrar\n");
  nPrintf("4: silvia solicita entrar\n");
  nPrintf("5: pedro solicita entrar\n");
  nPrintf("6: maria sale\n");
  nPrintf("7: ana sale\n");
  nPrintf("7: juan entra, pedro entra,\n");
  nPrintf("8: juan sale\n");
  nPrintf("9: pedro sale, silvia entra\n");
  nPrintf("10: silvia sale\n");
  nPrintf("==>\n");
  tiempo_actual= 1;
  maria= nEmitTask(humano, "maria", DAMA, 5, &t_maria);
  pausa(1);
  ana= nEmitTask(humano, "ana", DAMA, 5, &t_ana);
  pausa(1);
  juan= nEmitTask(humano, "juan", VARON, 1, &t_juan);
  pausa(1);
  silvia= nEmitTask(humano, "silvia", DAMA, 1, &t_silvia);
  pausa(1);
  pedro= nEmitTask(humano, "pedro", VARON, 2, &t_pedro);
  nWaitTask(juan);
  nWaitTask(pedro);
  nWaitTask(ana);
  nWaitTask(maria);
  nWaitTask(silvia);
  verificar(&t_maria, 1, 6, "maria");
  verificar(&t_ana, 2, 7, "ana");
  verificar(&t_juan, 7, 8, "juan");
  verificar(&t_silvia, 9, 10, "silvia");

  nPrintf("--- 8vo. test ------------------------------------------\n");
  nPrintf("1: juan solicita entrar, 1: juan entra,\n");
  nPrintf("2: maria solicita entrar, 3: pedro solicita entrar,\n");
  nPrintf("4: ana solicita entrar, 5: diego solicita entrar,\n");
  nPrintf("6: silvia solicita entrar, 7: juan sale\n");
  nPrintf("7: maria entra, 7: ana entra, 7: silvia entra,\n");
  nPrintf("8: ana sale, 9: silvia sale, 10: maria sale,\n");
  nPrintf("10: pedro entra, 10: diego entra,\n");
  nPrintf("11: pedro sale, 12: diego sale\n");
  nPrintf("==>\n");
  tiempo_actual= 1;
  juan= nEmitTask(humano, "juan", VARON, 6, &t_juan);
  pausa(1);
  maria= nEmitTask(humano, "maria", DAMA, 3, &t_maria);
  pausa(1);
  pedro= nEmitTask(humano, "pedro", VARON, 1, &t_pedro);
  pausa(1);
  ana= nEmitTask(humano, "ana", DAMA, 1, &t_ana);
  pausa(1);
  diego= nEmitTask(humano, "diego", VARON, 2, &t_diego);
  pausa(1);
  silvia= nEmitTask(humano, "silvia", DAMA, 2, &t_silvia);
  nWaitTask(pedro);
  nWaitTask(juan);
  nWaitTask(diego);
  nWaitTask(ana);
  nWaitTask(maria);
  nWaitTask(silvia);
  verificar(&t_juan, 1, 7, "juan");
  verificar(&t_maria, 7, 10, "maria");
  verificar(&t_ana, 7, 8, "ana");
  verificar(&t_silvia, 7, 9, "silvia");
  verificar(&t_pedro, 10, 11, "pedro");
  verificar(&t_diego, 10, 12, "diego");
  
  nPrintf("--- 9no. test ------------------------------------------\n");
  nPrintf("1: juan solicita entrar y juan entra\n");
  nPrintf("2: maria solicita entrar\n");
  nPrintf("3: pedro solicita entrar\n");
  nPrintf("4: ana solicita entrar\n");
  nPrintf("5: juan sale, maria y ana entran\n");
  nPrintf("6: silvia solicita entrar\n");
  nPrintf("7: diego solicita entrar\n");
  nPrintf("8: ana sale\n");
  nPrintf("9: maria sale, pedro y diego entran\n");
  nPrintf("10: pedro sale\n");
  nPrintf("11: paco solicita entrar\n");
  nPrintf("12: diego sale y silvia entra\n");
  nPrintf("13 silvia sale y paco entra\n");
  nPrintf("14: paco sale\n");
  nPrintf("==>\n");
  tiempo_actual= 1;
  juan= nEmitTask(humano, "juan", VARON, 4, &t_juan); // T=1
  pausa(1);
  maria= nEmitTask(humano, "maria", DAMA, 4, &t_maria); // T=2
  pausa(1);
  pedro= nEmitTask(humano, "pedro", VARON, 1, &t_pedro); // T=3
  pausa(1);
  ana= nEmitTask(humano, "ana", DAMA, 3, &t_ana); // T=4
  pausa(2);
  silvia= nEmitTask(humano, "silvia", DAMA, 1, &t_silvia); // T=6
  pausa(1);
  diego= nEmitTask(humano, "diego", VARON, 3, &t_diego); // T=7
  pausa(4);
  paco= nEmitTask(humano, "paco", VARON, 1, &t_paco); // T=11
  nWaitTask(juan);
  nWaitTask(maria);
  nWaitTask(pedro);
  nWaitTask(ana);
  nWaitTask(silvia);
  nWaitTask(diego);
  nWaitTask(paco);
  verificar(&t_juan, 1, 5, "juan");
  verificar(&t_maria, 5, 9, "maria");
  verificar(&t_pedro, 9, 10, "pedro");
  verificar(&t_ana, 5, 8, "ana");
  verificar(&t_silvia, 12, 13, "silvia");
  verificar(&t_diego, 9, 12, "diego");
  verificar(&t_paco, 13, 14, "paco");

  nPrintf("--- 10mo. test -------------------------------------------\n");
  nPrintf("1: maria solicita entrar,maria entra\n");
  nPrintf("2: ana solicita entrar, ana entra\n");
  nPrintf("3: juan solicita entrar\n");
  nPrintf("4: ana sale\n");
  nPrintf("5: maria sale, juan entra\n");
  nPrintf("6: pedro solicita entrar, pedro entra\n");
  nPrintf("7: pedro sale\n");
  nPrintf("8: juan sale\n");
  nPrintf("==>\n");
  tiempo_actual= 1;
  maria= nEmitTask(humano, "maria", DAMA, 4, &t_maria);
  pausa(1);
  ana= nEmitTask(humano, "ana", DAMA, 2, &t_ana);
  pausa(1);
  juan= nEmitTask(humano, "juan", VARON, 3, &t_juan);
  pausa(3);
  pedro= nEmitTask(humano, "pedro", VARON, 1, &t_pedro);
  nWaitTask(juan);
  nWaitTask(ana);
  nWaitTask(maria);
  nWaitTask(pedro);
  verificar(&t_maria, 1, 5, "maria");
  verificar(&t_ana, 2, 4, "ana");
  verificar(&t_juan, 5, 8, "juan");
  verificar(&t_pedro, 6, 7, "pedro");
  
  nPrintf("----------------\n");
  nPrintf("test de esfuerzo\n");
  verbose= FALSE;
  nSetTimeSlice(1);
  {
#   define NTASKS 3000
#   define NITER  1000
    nTask humanos[NTASKS];
    Tiempos h_tiempos[NTASKS];
    int mascara_sexo= 0xb38d2f0c;
    int i, j;
    for (j= 0; j<NTASKS; j++)
      humanos[j]= 0;
    for (i=0; i<NITER; i++) {
      int sexo;
      for (j= 0; j<NTASKS; j++)  {
        sexo= mascara_sexo>=0 ? VARON : DAMA;
        if (humanos[j]!=NULL)
          nWaitTask(humanos[j]);
        humanos[j]= nEmitTask(humano, "", sexo, 0, &h_tiempos[j]);
        mascara_sexo= (mascara_sexo<<1) | sexo;
      }
      if (i%11==0)
        nPrintf("%c", sexo ? 'F' : 'M');
    }
    for (j= 0; j<NTASKS; j++)
      nWaitTask(humanos[j]);
    nPrintf("\n\n\n");
  }

  nPrintf("Felicitaciones: paso todos los tests.\n");

  return 0;
}
