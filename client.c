#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define Rojo "\x1b[31m"
#define Verde "\x1b[32m"
#define Amarillo "\x1b[33m"
#define Azul "\x1b[34m"
#define Morado "\x1b[35m"
#define Celeste "\x1b[36m"
#define Blanco  "\x1B[37m"
#define ResetColor "\x1B[0m"  
 
//Argumento de creacion del hilo. Biblioteca que permite trabajar con 
//distintos hilos de ejecucion al mismo tiempo. 
pthread_t thread; 
 
void error (char *msg)
{
	perror(msg);
	exit(0);
}

void* receive(void* d)
{
	int sockfd = *((int *)d); //Establece el socket con una conexion al dominio en internet
	int a;
	char buf[256];
	
	
	while(1)
	{
		//Funcion que pone en 0 los 256 bytes que contiene el char buf
		bzero(buf,256);
		//Recibe el mensaje que contiene el socket
		a = recv(sockfd,buf,255,0);
		//Ocurre un error en la lectura del socket
		if(a <= 0)
			error("Error reading from socket");
		//Imprime el contenido del mensaje
		else
			printf(Azul"\n %s\n "Rojo,buf);
		
	}
	//cierra el socket
	close(sockfd);
	//Destruye el hilo
	pthread_exit(NULL);
}
int main(){
	sockets();
	}

int sockets () 
{
	int sockfd, portno, n,newsockfd,userno;
	char user[100],pass[30];
	struct sockaddr_in addr_in, serv_addr;
	struct hostent *server;
	char mensaje[250];
	char buffer[256];
	FILE *file;
	int verifica;
	char usuario[10]; 
	char puerto[10];
	char IP[30];
	file = fopen("config_client.txt","r");
 	fscanf(file,"%s",puerto);
 	fscanf(file,"%s",IP);
    fclose(file);
	
	//convierte el parametro puerto a entero
	portno = atoi(puerto);
	//Se recibe el dominio, el tipo, el protocolo para el socket
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	//Error al abrir el socket
	if (sockfd < 0)
		error("ERROR Opening Socket");
		
	//Toma el IP del servidor
    server = gethostbyname(IP);
    
    //Caso de que el servidor no existe
	if (server == NULL) 
	{
		fprintf(stderr, "ERROR, No Such Host\n");
		exit(0);
    }
    
    /*Establece la estructura de la direccion. Indica al socket donde
     debe conectarse*/
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    //Convierte el numero de puerto para la orden de red*/
    serv_addr.sin_port = htons(portno);
    /*Establece la conexion con el servidor*/
    newsockfd = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
    
    //Error al conectarse
    if (newsockfd < 0)
		error("ERROR de conexion");
    else
	{
		reg:
		bzero(user,100);
		//Solicitud de nombre de usuario
		username:
		printf(Morado"\nIngrese el nombre de usuario  : ");
		fgets(user,100,stdin);
		
		//Evalua el formato del usuario.strcmp(compara los strings) 
		if(strncmp(user,"",0)!= 0)
		{
			printf("USERNAME FORMAT NOT CORRECT  (FORMAT - user-<no>)");
			//Vuelve a solicitar nombre de usuario
			goto username;
		}
		//Asigna y envia el socket que contiene el usuario
		n = send(sockfd, user, strlen(user), 0);
		printf(Morado"\nIngrese la contrasena : ");
		//Lee la contrasena
		fgets(pass,30,stdin);
		//Asigna y envia el socket que contiene la contrasena
		n = send(sockfd, pass, strlen(pass), 0);
		bzero(buffer,256);
		n = recv(sockfd,buffer,255,0);
		
		if(n > 0)
			printf("%s\n",buffer);
		if(strcmp(buffer,"El usuario existe") == 0)
			goto reg;
	}
	//Abre el archivo y muestra los usuarios loggeados
	file = fopen("user.txt","a+");
	char str[100];
	bzero(str,100);
	char contacto[10];
	printf("\nUsuarios en linea:\n");
	
	while(fscanf(file,"%s",str) != EOF)
	{
		if((str != "server"))
			printf("\n%s\n",str);
		fscanf(file,"%s",str);
	}
	if(strcmp(buffer,"Usuario registrado")== 0)
	{
		fprintf(file,"%s",user);
		fprintf(file,"\n%s\n",pass);
	}
	fclose(file);
	pthread_create(&thread,NULL,receive,(void*)&sockfd);
	verifica = 0;
	
	//No sale de este ciclo hasta que el usuario indique: Adios
	while(1)
	{
		if (verifica == 0)
		{
			//Agrega el usuario con el que desea hablar
			printf(Celeste"Con quien desea chatear?");
			bzero (usuario, 10);
			fgets(usuario, 10, stdin);
			strcat(usuario,":");
			verifica++;
		}												
		enter:
		printf(Rojo"\nIngrese el mensaje: ");
		bzero(buffer, 256);
		printf(Rojo);
		fgets(buffer, 255, stdin);
  
		if((strncmp(buffer,"\0",1)!= 0)||(strncmp(buffer,"adios",5)== 0))
		{
			sprintf(buffer,"%s|%s",buffer,user);
			bzero(mensaje, 250);
			strcpy(mensaje,usuario);
			strcat(mensaje,buffer);
			n = send(sockfd, mensaje, strlen(mensaje), 0);
			 
			if(n<0)
				error("ERROR Writing To Socket");
			
			if(strncmp(buffer,"adios",5) == 0)
				break;   
		}
		else
		{
			printf("\n Formato del mensaje incorrecto\n");
			goto enter;
		}
	}
    close(sockfd); 
	return 0;
}
