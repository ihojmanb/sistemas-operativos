#ifndef _NSYSTEM_H_
#define _NSYSTEM_H_

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * Aqui se definen los prototipos de las operaciones basicas de
 * nSystem.  Los identificadores de proceso corresponden al
 * tipo de datos abstracto `nTask'.  En este archivo se define
 * como `void*' con el fin de lograr un tipo opaco, es decir
 * que solo es manipulable con las operaciones que aqui se definen.
 * Ademas se evita `marear' al programador que usa nSystem con
 * la implementacion de nSystem.
 *
 * Las verdaderas definiciones de nTask, nSem, nMonitor y nCondition
 * se encuentran en nSysimp.h u otros archivos y corresponden
 * a punteros a estructuras que contienen variada informacion.
 */

  typedef struct Task *nTask;
  typedef struct nSem *nSem;

  typedef struct nMonitor *nMonitor;
  typedef struct nCondition *nCondition;

/*************************************************************
 * Moléculas de agua
 *************************************************************/

typedef struct {
  void *oxy, *hydro1, *hydro2;
} *nH2o;

nH2o nCombineOxy(void *oxy, int timeout);
nH2o nCombineHydro(void *hydro);

/*************************************************************
 * La tarea principal provista por el programador
 *************************************************************/

int nMain( /* int argc, char **argv */ );

/* (No siempre sera necesario colocar argc y argv, por ello
 * los argumentos estan comentados) */

/*************************************************************
 * Creacion y manejo de tareas
 *************************************************************/

typedef int (*nProc)();
nTask nEmitTask(nProc, ... );
                              // Crea una nueva tarea */
void nExitTask(int rc);       // Termina la tarea que la invoca */
int nWaitTask(nTask task);    // Espera el termino de otra tarea */

void nExitSystem(int rc);     // Termina todas las tareas (el proceso Unix)

// Para poder pasar reales como parametros
long long Real2LL(double rv);
double LL2Real(long long llv);

/*************************************************************
 * Definicion de parametros para las tareas
 *************************************************************/

int  nSetStackSize(int size);  // Taman~o de stack para las nuevas tareas */
void nSetTimeSlice(int slice); // Taman~o de la tajada (en ms)
void nSetTaskName(char *format, ... ); // Util para debugging

nTask nCurrentTask(void);      // El identificador de la tarea actual
char* nGetTaskName(void );     // El nombre de esta tarea
int nGetContextSwitches(void); // Cambos de contexto implicitos
int nGetQueueLength();         // Numero de procesos en ready queue

/*************************************************************
 * Mensajes
 *************************************************************/

int nSend(nTask task, void *msg); // Envia un mensaje a una tarea
void *nReceive(nTask *ptask, int max_delay);
                                  // Recepcion de un mensaje
void nReply(nTask task, int rc);  // Responde un mensaje

void nSleep(int delay);           // Suspende el proceso por delay milisecs
int nGetTime(void);               // La hora en milisegundos y modulo 'maxint'

/*************************************************************
 * Semaforos
 *************************************************************/

nSem nMakeSem(int count);   // Construye un semaforo
void nWaitSem(nSem sem);    // Operacion Wait
void nSignalSem(nSem sem);  // Operacion Signal
void nDestroySem(nSem sem); // Destruye un semaforo

/*************************************************************
 * Monitores
 * Cuidado! Estos monitores no son reentrantes
 *************************************************************/

nMonitor nMakeMonitor();             // Construye un monitor
void nDestroyMonitor(nMonitor mon);  // Destruye un monitor
void nEnter(nMonitor mon);           // Ingreso al monitor
void nExit(nMonitor mon);            // Salida del monitor
void nWait(nMonitor mon);            // Libera el monitor y suspende
void nNotifyAll(nMonitor mon);       // Retoma tareas suspendidas

nCondition nMakeCondition(nMonitor mon); // Construye una condicion
void nDestroyCondition(nCondition cond); // Destruye una condicion
void nWaitCondition(nCondition cond);    // operacion Wait
void nSignalCondition(nCondition cond);  // operacion Signal

/*************************************************************
 * E/S basica
 *************************************************************/

/* Estas funciones son equivalentes a open, close, read y write en
 * Unix.  Las 'nano' funciones son no bloqueantes para el proceso Unix,
 * solo bloquean la tarea que las invoca.
 */

int nOpen( char *path, int flags, ... );    // Abre un archivo
int nClose(int fd);                         // Cierra un archivo
int nRead(int fd, char *buf, int nbyte);    // Lee de un archivo
int nWrite(int fd, char *buf, int nbyte);   // Escribe en un archivo

/* Estas funciones se pueden usar en caso de necesitar que
 * la E/S estandar sea no bloqueante (ver examples1/iotest.c).
 * (En algunos casos como curses, es inevitable que sea bloqueante).
 */

void nReopenStdio(void);         // Reabre la E/S estandar
void nSetNonBlockingStdio(void); // Coloca en modo no bloqueante la E/S std.

/*************************************************************
 * Los servicios
 *************************************************************/

int nFprintf( int fd, char *format, ... );
int nPrintf( char *format, ... );
void nFatalError( char *procname, char *format, ... );

void *nMalloc(int size);
void nFree(void *ptr);

/*************************************************************
 * Constantes varias
 *************************************************************/

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef EOF
#define EOF (-1)
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define nAssert(a, msg) if (!a) { nFatalError("Assertion failure", msg); } else ;

#endif   /* _NSYSTEM_H_ */
