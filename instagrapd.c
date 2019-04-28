#include <stdio.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

int port ;
char ip[16] ;
int wport ;
char dir[128] ;

typedef struct student * pStudent ;
typedef struct student {
	char id[10] ;
	char pwd[10] ;
	pStudent next ; 
} student ; 

pStudent head = NULL ;

void 
option_handler(int argc, char*argv[])
{

	char opt ;
	char * tok ;

	if (argc != 6)
	{
		printf("\t protocol : ./instagrapd -p <Port> -w <IP>:<Wport> <Dir>    \n") ;
		printf("\t all arguments are required. \n");
		exit(0);
	}

	
	strcpy(dir, argv[5]) ;

	while ( (opt = getopt(argc, argv, "p:w:")) != -1) 
	{
		switch(opt)
		{
			case 'p' :
				port = atoi(optarg) ;
				break;

			case 'w' :
				tok = strtok(optarg,":") ;
				strcpy(ip,tok) ;
				if( tok != NULL) 
					tok = strtok(NULL,":");	
				wport = atoi(tok) ;	
				break;
			default : break ;	
		}
	}

}

int
authenticate(char * data)
{
	char * tok ;
	char curr_id[10] ;
	char curr_pwd[10] ;
	tok = strtok(data,"-") ;
	strcpy(curr_id,tok) ;
	if( tok != NULL)
		tok = strtok(NULL,"-") ;
	strcpy(curr_pwd,tok);
	
// for debug
printf("id: %s , pwd : %s \n",curr_id,curr_pwd) ; 	

	if(head == NULL) {
printf("head is NULL?\n");	
		pStudent stu = (pStudent)malloc(sizeof(student)) ;
		strcpy(stu->id,curr_id);
		strcpy(stu->pwd,curr_pwd) ;
		stu->next = NULL ;
		head = stu ;
printf("head->id: %s\n", head->id) ;
		return 1 ; 
	}
	pStudent aStudent = head;
	while(aStudent != NULL){
		if(strcmp(aStudent->id,curr_id) == 0) {
			if(strcmp(aStudent->pwd, curr_pwd) == 0) return 1 ;
			else return 0 ; 
		}
		aStudent = aStudent->next ; 
	}

printf("new id? \n");	
	pStudent stu = (pStudent)malloc(sizeof(student)) ;
	strcpy(stu->id, curr_id) ;
	strcpy(stu->pwd, curr_pwd) ;
	stu->next = head ;
	head = stu ;

	return 1 ;
}

void 
send_to_worker(char * file)
{
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
	serv_addr.sin_port = htons(wport /*wport*/); 
	if (inet_pton(AF_INET, ip /*ip*/ , &serv_addr.sin_addr) <= 0) {
		perror("inet_pton failed : ") ; 
		exit(EXIT_FAILURE) ;
	} 

	if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("connect failed : ") ;
		exit(EXIT_FAILURE) ;
	}

	/*scanf("%s",buffer)*/
	sprintf(buffer,"%s-%s",file,dir) ;

	data = buffer ;
	len = strlen(buffer) ;
	s = 0 ;
	while (len > 0 && (s = send(sock_fd, data, len, 0)) > 0) {
		data += s ;
		len -= s ;
	}

	shutdown(sock_fd, SHUT_WR) ;


}



void
child_proc(int conn)
{
	char buf[1024] ;
	char * data = 0x0, * orig = 0x0 ;
	char * aut_msg = 0x0 ;
	int len = 0 ;
	int s ;

	while ( (s = recv(conn, buf, 1023, 0)) > 0 ) {
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
	printf(">%s\n", data) ; // data : 21700696-abcd1234
		
	orig = data ;

	if(strstr(data,".c") != 0x0) {   
		send_to_worker(data) ;	
	}
	else {  // -----------------------not file part start

// for debug
printf("come by here \n");

	if(authenticate(data)) 	
		aut_msg = strdup("accept") ;
	else aut_msg = strdup("reject") ;

// for debug
printf("come here too \n") ;

	while (len > 0 && (s = send(conn, aut_msg, len, 0)) > 0) {
		aut_msg += s ;
		len -= s ;
	}


	shutdown(conn, SHUT_WR) ; 

	} 
// -------------------------------------------- not file part finished 
	if (orig != 0x0) 
		free(orig) ;

}

int 
main(int argc, char *argv[])
{
	option_handler(argc, argv) ;

	int listen_fd, new_socket ;
	struct sockaddr_in address; 
	int addrlen = sizeof(address); 

	char buffer[1024] = {0}; 

	listen_fd = socket(AF_INET /*IPv4*/, SOCK_STREAM /*TCP*/, 0 /*IP*/) ;
	if (listen_fd == 0)  { 
		perror("socket failed : "); 
		exit(EXIT_FAILURE); 
	}
	
	memset(&address, '0', sizeof(address)); 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY /* the localhost*/ ; 
	address.sin_port = htons(port); 
	if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed : "); 
		exit(EXIT_FAILURE); 
	} 

	while (1) {
		if (listen(listen_fd, 16 /* the size of waiting queue*/) < 0) { 
			perror("listen failed : "); 
			exit(EXIT_FAILURE); 
		} 

	new_socket = accept(listen_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen) ;
		if (new_socket < 0) {
			perror("accept"); 
			exit(EXIT_FAILURE); 
		} 

		if (fork() > 0) {
			child_proc(new_socket) ;
		}
		else {
			close(new_socket) ;
		}
	}

}






















