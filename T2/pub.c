#include <nSystem.h>
#include "fifoqueues.h"
#include "pub.h"

typedef struct{
    nMonitor m; // monitor
    FifoQueue v; // cola varones
    FifoQueue d; // cola damas
	int varones, damas; // contadores
}Ctrl;

typedef struct {
	int sexo; // VARON | DAMA
	nCondition w; // en la condición voy a esperar
	int ready; // inicialmente false, indica si la persona puede entrar al baño
} Request;

Ctrl* c; // estructura que encapsula las variables globales

void await(Ctrl *c, int s){
	Request r = {s, nMakeCondition(c->m), FALSE};
    if(s == VARON){
	    PutObj(c->v, &r);
    }
    else{
        PutObj(c->d, &r);
    }
	while(!r.ready) // ojo es r. pues r NO es un puntero
		nWaitCondition(r.w);
	nDestroyCondition(r.w); // esto evita la gotera de memoria
}

void ini_pub() {
    c = (Ctrl*)nMalloc(sizeof(Ctrl));
    c->m = nMakeMonitor();
    c->v = MakeFifoQueue(); // varones esperando a entrar
    c->d = MakeFifoQueue(); // damas esperando a entrar
    c->varones = 0; // varones en el baño
    c->damas = 0;   // damas en el baño
}

void entrar(int sexo){
    nEnter(c->m);
    if(sexo==VARON){
        //si hay damas adentro o hay damas esperando antes que varon
        if(c->damas != 0 || !EmptyFifoQueue(c->d)){
            await(c, VARON);
        }else{
            c->varones++;
        }
    }else{
        //si hay varones adentro o hay varones esperando antes que dama
        if(c->varones != 0 ||!EmptyFifoQueue(c->v)){
            await(c, DAMA);
        }else{
            c->damas++;
        }
        
    }
    nExit(c->m);

}
void salir(int sexo){
    nEnter(c->m);
    if(sexo==VARON){
        if(c->varones > 1){
            c->varones--;
        }
        // si es el ultimo varon
        else if(c->varones == 1){
            c->varones--;
            while(!EmptyFifoQueue(c->d)){
                Request *r = GetObj(c->d);
                r->ready = TRUE;
                nSignalCondition(r->w);
                c->damas++;
            }
        }
    }else{
        if(c->damas > 1){
            c->damas--;
        }
        // si es la ultima dama
        else if(c->damas == 1){
            c->damas--;
            while(!EmptyFifoQueue(c->v)){
                Request *r = GetObj(c->v);
                r->ready = TRUE;
                nSignalCondition(r->w);
                c->varones++;
            }
        }
        
    }
    nExit(c->m);
}