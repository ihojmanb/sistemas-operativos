# Descargue el nSystem desde:
# https://users.dcc.uchile.cl/~lmateu/CC4302/download/nsystem64-beta2.tgz
# Ud. debe descomprimir este archivo con:
# $ tar zxvf nsystem64-beta2.tgz
# Compile el nSystem con:
# $ cd nsystem64-beta2/src
# $ make

# Importante:
# Para compilar exitosamente su tarea necesita definir la variable de
# ambiente NSYSTEM con:
# $ export NSYSTEM=.../nsystem64-beta2
# (debe reemplazar ... por la ruta para llegar a nsystem64-beta2)

# Programe su tarea en el archivo equipo.c y luego pruebela con:
# $ cd <el directorio en donde se encuentra este archivo>
# $ make test-equipo
# $ ./test-equipo

LIBNSYS= $(NSYSTEM)/lib/libnSys.a

# Con opciones de debugging
CFLAGS= -Wall -pedantic -std=c99 -g -I$(NSYSTEM)/include -I$(NSYSTEM)/src
LDFLAGS= -g
LDLIBS= $(LIBNSYS)

# Con opciones de optimizacion:
#CFLAGS= -O2 -I$(NSYSTEM)/include -I$(NSYSTEM)/src

test-equipo: test-equipo.o equipo.o

test-equipo.o equipo.o equipo-enun.o: equipo.h

test-enun: test-equipo.o equipo-enun.o
	gcc $(LDFLAGS) test-equipo.o equipo-enun.o $(LDLIBS) -o test-enun

test-inc: test-equipo.o equipo-inc.o
	gcc $(LDFLAGS) test-equipo.o equipo-inc.o $(LDLIBS) -o test-inc

clean:
	rm -f *.o test-equipo test-inc test-enun
