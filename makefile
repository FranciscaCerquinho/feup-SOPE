all: sauna gerador

sauna: sauna.o
	gcc -pthread sauna.o -o sauna

gerador: gerador.o
	gcc -pthread gerador.o -o gerador

sauna.o: sauna.c pedido.h
	gcc -pthread -Wall -c sauna.c

gerador.o: gerador.c
	gcc -pthread -Wall -c gerador.c

clean:
	rm -f sauna gerador \
		sauna.o gerador.o