# Instrucciones de uso:
# - Primero defina la variable LIBNSYS con la ubicacion de nSystem
#   en su computador.
# - Compile con: 
#   % make
#   ...
# - Ejecute con:
#   % ./test-exchange
# 
# No entregue en U-cursos tareas que no pasan todos los tests.
# 
# La ultima serie de tests mide tiempos y por lo tanto pueden
# fallar en una maquina con mucha carga.  En ese caso repita
# la ejecucion en una maquina con poca carga.
# Si el error persiste, es su implementacion la que tiene problemas.


LIBNSYS= $(NSYSTEM)/lib/libnSys.a

CFLAGS= -ggdb -Wall -pedantic -std=c99 -I$(NSYSTEM)/include -I$(NSYSTEM)/src
LFLAGS= -ggdb

all: test-exchange

.SUFFIXES:
.SUFFIXES: .o .c .s

.c.o .s.o:
	gcc -c $(CFLAGS) $<

test-exchange: test-exchange.o $(LIBNSYS)
	gcc $(LFLAGS) test-exchange.o -o $@ $(LIBNSYS)

clean:
	rm -f *.o *~

cleanall:
	rm -f *.o *~ test-exchange
