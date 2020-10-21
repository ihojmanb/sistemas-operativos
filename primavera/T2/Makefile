# Importante:
# Para compilar exitosamente su tarea necesita definir la variable de
# ambiente NSYSTEM con:
# $ export NSYSTEM=.../nsystem64-beta2
# (debe reemplazar ... por la ruta para llegar a nsystem64-beta2)

# Programe su tarea en el archivo pub.c y luego pruebela con:
# $ cd <el directorio en donde se encuentra este archivo>
# $ make test-g
# $ make test-O

LIBNSYS= $(NSYSTEM)/lib/libnSys.a

# Con opciones de debugging
CFLAGS= -Wall -pedantic -std=c99 -g -I$(NSYSTEM)/include -I$(NSYSTEM)/src
LDFLAGS= -g
LDLIBS= $(LIBNSYS) -lpthread

# Para probar con pSystem (nSystem implementado a partir de pthreads)
# LDLIBS= $(LIBNSYS) -lpthread

# Con opciones de optimizacion:
# CFLAGS= -O2 -I$(NSYSTEM)/include -I$(NSYSTEM)/src
# LDFLAGS=

all: test-pub

test-pub.o test2-pub.o test3-pub.o pub.o: pub.h

test-pub: test-pub.o pub.o

test-g:
	make clean
	make
	bash -c 'time ./test-pub'

test-O:
	make clean
	make CFLAGS="-O2 -I$(NSYSTEM)/include -I$(NSYSTEM)/src"
	bash -c 'time ./test-pub'

clean:
	rm -f *.o test-pub
