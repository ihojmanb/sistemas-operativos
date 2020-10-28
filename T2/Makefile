# Importante:
# Para compilar exitosamente su tarea necesita definir la variable de
# ambiente NSYSTEM con:
# $ export NSYSTEM=.../nsystem64-beta2
# (debe reemplazar ... por la ruta para llegar a nsystem64-beta2)

# Programe su tarea en el archivo pub.c y luego pruebela con:
# $ cd <el directorio en donde se encuentra este archivo>
# $ make test-g
# $ make test-O
#
# Las siguientes son otras formas de ejecutar su tarea para detectar
# errores de manejo de memoria o dataraces.  Pero *debe* usar pSystem64.
# No intente ejecutar valgrind, drd o sanitize con nSystem64.  Obtendria
# demasiados reportes falso positivos.
#
# make run-valgrind-ddd
# Lanza un proceso para que pueda ser depurado con ddd con el comando:
# target remote | vgdb --pid ...pid...
# Hay reemplazar ...pid... por el numero que reporta valgrind con ===numero===
# Si valgrind detecta un error de manejo de la memoria la ejecucion se detiene
# justo antes que ocurra y se puede usar ddd para revisar los valores de las
# variables del proceso.
#
# make run-drd-ddd
# Similar a run-valgrind-ddd pero chequea dataraces en vez de errores
# de manejo de memoria.  La ejecucion es extremadamente lenta.
# Es bueno alterna run-valgrind-ddd y run-drd-ddd porque a veces un
# error de manejo de memoria gatilla un datarace y a veces un datarace
# gatilla un error de manejo de memoria
#
# make run-sanitize
# Al igual que run-drd-ddd sirve para detectar dataraces pero la ejecucion
# es mucho mas rapida.  La desventaja es que no se puede usar con ddd y
# ademas *hay que recompilar* pSystem64 con make sanitize.

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

all: clean test-pub

test-g:
	make
	./test-pub

test-O: optimize
	./test-pub

test-pub.o pub.o: pub.h $(NSYSTEM)/include/nSystem.h $(NSYSTEM)/src/fifoqueues.h

test-pub: test-pub.o pub.o $(LIBNSYS)

debug:
	make

optimize:
	make CFLAGS="-O -Wall -pedantic -std=c99 -I$(NSYSTEM)/include -I$(NSYSTEM)/src" LDFLAGS=""

run-valgrind:
	make CFLAGS="-g -DVALGRIND=1 -Wall -pedantic -std=c99 -I$(NSYSTEM)/include -I$(NSYSTEM)/src"
	bash -c 'valgrind ./test-pub |& tee test-pub.log'

run-valgrind-ddd:
	make CFLAGS="-g -DVALGRIND=1 -Wall -pedantic -std=c99 -I$(NSYSTEM)/include -I$(NSYSTEM)/src"
	bash -c 'valgrind --vgdb=yes --vgdb-error=0 ./test-pub |& tee run.log'

run-drd:
	make CFLAGS="-g -DVALGRIND=1 -Wall -pedantic -std=c99 -I$(NSYSTEM)/include -I$(NSYSTEM)/src"
	bash -c 'valgrind --tool=drd ./test-pub |& tee test-pub.log'

run-drd-ddd:
	make CFLAGS="-g -DVALGRIND=1 -Wall -pedantic -std=c99 -I$(NSYSTEM)/include -I$(NSYSTEM)/src"
	bash -c 'valgrind --tool=drd --vgdb=yes --vgdb-error=0 ./test-pub |& tee run.log'

run-sanitize:
	make CFLAGS="-DVALGRIND=1 -fsanitize=thread -Wall -pedantic -std=c99 -g -I$(NSYSTEM)/include -I$(NSYSTEM)/src" LDFLAGS="-g -fsanitize=thread"
	bash -c './test-pub |& tee run.log'

clean:
	rm -f *.o test-pub
