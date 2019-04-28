#include <stdio.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

char ip[16] ;
int port ;
char sid[10] ;
char pwd[10] ;
char file[32] ;

void option_handler(int argc, char *argv[])
{
	char opt;
	char * tok ;

	if (argc != 8)
	{
		printf("\t protocol : ./submitter -n <IP>:<Port> -u <ID> -k <PW> <File>\n") ;
		printf("\t all arguments are required. \n");
		return ;
	}
	
	strcpy(file, argv[7]) ;
	
	while((opt = getopt(argc, argv, "n:u:k:")) != -1)
	{
		switch(opt) 
		{
			case 'n' :
				tok = strtok(optarg,":") ;
				strcpy(ip,tok) ;
				if( tok != NULL) 
					tok = strtok(NULL,":");	
				port = atoi(tok) ;		
				
				break;

			case 'u' :
				if(strlen(optarg) == 8)
				{
				   for(int i=0; i<8 ; i++)
				     if(!isdigit(optarg[i]))
				     {
					printf("Error: student id should be 8 digit numbers \n") ;
					return ;
				     }  
				

				   strcpy(sid, optarg) ;
					
				}
				else
				{
				   printf("Error : student id should be 8 digit numbers\n") ;
				   return ;
				}
		
				break;

			case 'k' :	
				if(strlen(optarg) == 8)
				{
				   for(int i=0; i<8 ; i++)
					if(!isalnum(optarg[i]))
					{
					   printf("Error: password should be 8 alphanumeric. \n") ;
					   return ;
				        }

				   strcpy(pwd, optarg) ;
					
				}
				else 
				{
				   printf("Error : password should be 8 alphanumeric. \n") ;
				   return ;
				}		

				break;

			default : break;
		}
	}

		


}




int
main(int argc, char *argv[])
{
	option_handler(argc,argv) ;

	struct sockaddr_in serv_addr; 
	int sock_fd ;
	int s, len ;
	char buffer[1024] = {0}; 
	char * data ;
	
	sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
	if (sock_fd <= 0) {
		perror("socket failed : ") ;
		exit(EXIT_FAILURE) ;
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(port /*port*/); 
	if (inet_pton(AF_INET, ip /*ip*/ , &serv_addr.sin_addr) <= 0) {
		perror("inet_pton failed : ") ; 
		exit(EXIT_FAILURE) ;
	} 

	if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("connect failed : ") ;
		exit(EXIT_FAILURE) ;
	}

	/*scanf("%s",buffer)*/
	sprintf(buffer,"%s-%s",sid,pwd) ;


	data = buffer ;
	len = strlen(buffer) ;
	s = 0 ;
	while (len > 0 && (s = send(sock_fd, data, len, 0)) > 0) {
		data += s ;
		len -= s ;
	}

	shutdown(sock_fd, SHUT_WR) ;

	char buf[1024] ;
	data = 0x0 ;
	len = 0 ;
	while ( (s = recv(sock_fd, buf, 1023, 0)) > 0 ) {
		buf[s] = 0x0 ;
		if (data == 0x0) {
			data = strdup(buf) ;
			len = s ;
		}
		else {
			data = realloc(data, len + s + 1) ;
			strncpy(data + len, buf, s) ;
			data[len + s] = 0x0 ;
			len += s ;
		}

	}
	printf(">%s\n", data); // accept or reject

/*-------------------------------------------if accepted -----*/

	if(strcmp(data, "accept") == 0) {
		
	sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
	if (sock_fd <= 0) {
		perror("socket failed : ") ;
		exit(EXIT_FAILURE) ;
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(port /*port*/); 
	if (inet_pton(AF_INET, ip /*ip*/ , &serv_addr.sin_addr) <= 0) {
		perror("inet_pton failed : ") ; 
		exit(EXIT_FAILURE) ;
	} 

	if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("connect failed : ") ;
		exit(EXIT_FAILURE) ;

	}


	sprintf(buffer,"%s",file) ;

	data = buffer ;
	len = strlen(buffer) ;
	s = 0 ;
	while (len > 0 && (s = send(sock_fd, data, len, 0)) > 0) {
		data += s ;
		len -= s ;
	}

	shutdown(sock_fd, SHUT_WR) ;

}

}

// -----------------------------------------------------------------






