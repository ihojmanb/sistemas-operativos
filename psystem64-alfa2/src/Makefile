# Este archivo Makefile sirve para generar la biblioteca de nSystem
# ubicada en el ../lib/nSyslib.a
#
# Formas de compilacion:
# make
# Sin ningun parametro compila la version con -g para debugging
#
# make optimize
# Compila la version optimizada con -O
#
# make santize
# Compila la version con -g y ayuda para detectar dataraces
#
# make debug-m32
# Compila la version de 32 bits con -g para debugging
#
# make optimize-m32
# Compila la version de 32 bits optimizada con -O
#
# make clean
# Borra archivos .o sin borrar la biblioteca

# Archivos necesarios para la compilacion
NSYSTEMOBJS= psystem.o fifoqueues.o

# La ubicacion de la biblioteca de nSystem
LIBNSYS=../lib/libnSys.a

# Opciones de compilacion en caso de omision
CFLAGS= -g -Wall -pedantic -I../include

all: clean libnsys

debug:
	make libnsys

libnsys: $(NSYSTEMOBJS)
	rm -rf $(LIBNSYS)
	sh ar-ranlib $(LIBNSYS) $(NSYSTEMOBJS)

optimize:
	make CFLAGS="-O -Wall -pedantic -I../include"

sanitize:
	make CFLAGS="-fsanitize=thread -g -Wall -pedantic -I../include"

debug-m32:
	make CFLAGS="-g -m32 -Wall -pedantic -I../include"

optimize-m32:
	make CFLAGS="-O -m32 -Wall -pedantic -I../include"

clean:
	rm -f *.o *~
