
all: Servidor, Cliente


CPPFLAGS = -g -I.

Servidor : server.c
	cc -g -I. server.c -o Servidor

Cliente : client.c
	cc -g -I. client.c -o Cliente
