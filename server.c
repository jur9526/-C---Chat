#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

//Definicion de macros para el uso de colores
#define Rojo "\x1b[31m"
#define Verde "\x1b[32m"
#define Amarillo "\x1b[33m"
#define Azul "\x1b[34m"
#define Morado "\x1b[35m"
#define Celeste "\x1b[36m"
#define ResetColor "\x1b[0m" 
 
//Creacion del puntero para el manejo de archivos
FILE *fp;

//Estructura para almacenar el nombre de usuario y el sockets para cada usuario
struct userinfo
{
	char username[100];
	int socket;
}
u[1000]; 

//definicios de los hilos para manejar las diferentes tareas
pthread_t thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
 
//variables que se utilizan para el envio de mensajes
char buffer[256],user[100],str[100],pass[30];
struct sockaddr_in serv_addr, cli_addr;
int n,nu=1,i;

//Funcion que se encarga de los errores
void error(char *msg) 
{
	perror(msg); 
	exit(0);
}


//Se encarga del manejo de los mensajes
void* server(void*);
  
int main () 
{
	/*fp = fopen("user.txt","w");
	fprintf(fp,"server started\n");
	fclose(fp);*/
 
	int i,sockfd, newsockfd[1000], portno, clilen,no=0,n;
	char puerto[10];
	//abre el archivo de configuracion para obtener el puerto por utilizar
	fp = fopen("config_server.txt","r");
	fscanf(fp,"%s",puerto);
	fclose(fp);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	   
	if (sockfd<0) 
		error("ERROR! Cannnot Open Socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(puerto);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	   
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0)
		error("ERROR! On Binding");
	   
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	
	while(1)
	{
		newsockfd[no] = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		if (newsockfd<0)
			error("ERROR! On Accepting");
	 
		pthread_mutex_lock(&mutex);
		pthread_create(&thread,NULL,server,(void*)&newsockfd[no]);
		no+=1;
	}
	close(newsockfd[no]); 
	close(sockfd);
	return 0;
}

//funcion server definida al inicio 
void* server(void* sock)
{
	int newsockfd=*((int*)sock),j=0,m;
	char to[100],from[100],name[100];
	fp=fopen("user.txt","r+");
	//verifica la informacion de los usuarios
	checking:
	m=1;
	bzero(user,100);
	bzero(to,100);
	bzero(from,100);
	bzero(str,100);
	recv(newsockfd, user,100,0);
	recv(newsockfd, pass,30,0);
	while(fscanf(fp,"%s",str)!=EOF)
	{
		n= strncmp(user,str,strlen(str));
		if(n==0)
		{
			fscanf(fp,"%s",str);
			n= strncmp(pass,str,strlen(str));
			if(n==0)
			{
				m=2;
				break;
			}
			else
			{
				send(newsockfd,"El usuario existe",17,0);
				m=0;
				break;
			}
		fscanf(fp,"%s",str);
		}
	}
	if(m==0)
		goto checking;
	if(m==1)
	{
		fclose(fp);
		send(newsockfd,"Usuario registrado",18,0);
		bzero(u[nu].username,100);
		u[nu].socket=newsockfd;
		strncpy(u[nu].username,user,strlen(user));
		nu++;
	}
	if(m==2)
	{
		fclose(fp);
		send(newsockfd,"Usuario en linea",16,0);
		
		for(i=1;i<nu;i++)
		if(strcmp(user,u[i].username)==0)
			break;
		u[i].socket=newsockfd;
	}
	pthread_mutex_unlock(&mutex);
	 
	bzero(buffer, 256);
	int newsockfd1;
	
	while(1)
	{ 
		n = recv(newsockfd, buffer, 255, 0);
		if(n<0)
			error("ERROR! Reading From Socket");
		if(strncmp(buffer,"adios",5)==0)
		{
			close(newsockfd);
			pthread_exit(NULL);
		}
		else if(strncmp(buffer,"archivo:",8)){
			
			
			close(newsockfd);
			pthread_exit(NULL);
			}
		i=0;
		strcpy(name,buffer);
		
		while(name[i]!=':')
		{
			to[i] = name[i];
			i++;
		}
		printf("Esta es la palabra: " );
		puts(to); //Imprime el nombre de usuario (user-luis)
		to[i]='\0';
		
		printf("nueva version de to");
		puts(to);
		j=0;
		bzero(buffer,256);
		
		//Obtiene el mensaje
		while(name[i]!='|')
		{
			buffer[j]= name[i];
			i++;
			j++;
		}
		printf("Imprime nombre");
		puts(name);
		buffer[j]='\0';
		j=0;
		
		for(i+=1;name[i]!='\0';i++)
		{
			from[j]=name[i];
			j++;
		}
		
		from[j-1]='\0';
		printf("De: %s  Para: %s Message %s",to,from,buffer);
	  
		for(j=1;j<nu;j++)
		{
			if((strncmp(u[j].username,to,strlen(to)))==0)
			{
				newsockfd1=u[j].socket;
				break;
			}
		}
		strcat(from,buffer);
		bzero(buffer,256);
		//strcpy(buffer,);
		strcat(buffer,from); 
		
		n=send(newsockfd1,buffer,sizeof buffer,0);
		
		if(n<0)
		{
			send(newsockfd, "SENDING FAILED : USER LOGGED OUT",32,0); 
		}
		else
		{
			n = send(newsockfd, "", 0, 0);
			if (n<0)
				error("ERROR! Writing To Socket");
		}
	}
	close(newsockfd);
	pthread_exit(NULL);
}
