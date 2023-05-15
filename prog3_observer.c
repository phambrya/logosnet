/*
Bryan Pham
Project 3: LogosNet
Due: July 31, 2021
Description: This is an observer where it main purpose is to display the messages. This should be able to quit
and ask for an user name that has already been pick by a participant.
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

int main( int argc, char **argv) {
	struct hostent *ptrh; /* pointer to a host table entry */
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold an IP address */
	int sd; /* socket descriptor */
	int port; /* protocol port number */
	char *host; /* pointer to host name */
	int n; /* number of characters read */
	char buf[1000]; /* buffer for data from the server */

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./client server_address server_port\n");
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[2]); /* convert to binary */
	/* test for legal value */
	if (port > 0)
	{ 
		sad.sin_port = htons((u_short)port);
	}
	else {
		fprintf(stderr,"Error: bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}

	host = argv[1]; /* if host argument specified */

	/* Convert host name to equivalent IP address and copy to sad. */
	ptrh = gethostbyname(host);
	if ( ptrh == NULL ) {
		fprintf(stderr,"Error: Invalid host: %s\n", host);
		exit(EXIT_FAILURE);
	}

	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	/* Map TCP transport protocol name to protocol number. */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket. */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* TODO: Connect the socket to the specified server. You have to pass correct parameters to the connect function.*/
	if (connect(sd, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		fprintf(stderr,"connect failed\n");
		exit(EXIT_FAILURE);
	}

	
	

	int fdmax;
	fd_set master;
	fd_set read_fds;
	
	
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	FD_SET(sd, &master);
	FD_SET(STDIN_FILENO, &master);
	fdmax = sd;
	char send_buf[1000];

	int connected = 0; // 1 if yes
	int hasUsrName= 0; // 1 if yes
	int gaveClientType = 0;



	while(1){
		read_fds = master;
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
			perror("select");
			exit(4);
		}
		

		for(int i = 0; i <= fdmax; i++)
		{
			if(FD_ISSET(i, &read_fds))
			{
				if(i == STDIN_FILENO)
				{ //if console input
					fgets(send_buf, 1000, stdin);
					send_buf[strcspn(send_buf, "\n")] = 0;
					if(strcmp(send_buf, "/quit") == 0) 
					{
						close(sd);
						exit(0);
					}
				} 
				else 
				{
					char recv_buf[1000];
					memset(&recv_buf[0], 0, sizeof(recv_buf));
					int n = recv(i, recv_buf, sizeof(recv_buf), 0);
					if(hasUsrName == 0)
					{
						//add client to server
						if(connected == 0)
						{
							if(recv_buf[0] == 'Y')
							{
								//connect client
								connected = 1;
								uint16_t clientType = 1; // send 1 if participant  
								int user_tf = 0;
								uint16_t msgSizeOut;
								char temp_buf[15];
								char name_buf[10];
								while(user_tf == 0)
								{
									fd_set input;
									struct timeval timeout;
									int check_input = 0;
									uint8_t input_bytes = 0;
									uint8_t time = 10;

									FD_ZERO(&input);
									FD_SET(0, &input);

									timeout.tv_sec = time;
									timeout.tv_usec = 0;

									printf("Enter a Username: \n");

									check_input = select(1, &input, NULL, NULL, &timeout);
									if(check_input == -1)
									{
										perror("select is unable to read your input\n");
									}
									if(check_input == 0)
									{
										//printf("close");
										msgSizeOut = 0;
										send(sd, &msgSizeOut, sizeof(uint16_t), 0);
										close(sd);
										return 1;
									}
									if(check_input) 
									{
										memset(&temp_buf[0], 0, sizeof(temp_buf));
										fgets(temp_buf, 15, stdin);
										//memset(&name_buf[0], 0, sizeof(name_buf));
										//strncpy(name_buf, temp_buf, strlen(temp_buf)-1);
									}

									//fgets(name_buf, 12, stdin);
									//printf("Name: ----%s----\n", name_buf);
									msgSizeOut = strlen(temp_buf) -1;
									//printf("length: %d", msgSizeOut);
									if(msgSizeOut <= 10)
									{
										memset(&name_buf[0], 0, sizeof(name_buf));
										strncpy(name_buf, temp_buf, strlen(temp_buf)-1);
										for(int i = 0; i < msgSizeOut; i++)
										{
											if(name_buf[i] >= 'A' && name_buf[i] <= 'Z')
											{
												user_tf = 1;
											}
											else if(name_buf[i] >= 'a' && name_buf[i] <= 'z')
											{
												user_tf = 1;
											}
											else if(name_buf[i] >= '0' && name_buf[i] <= '9')
											{
												user_tf = 1;
											}
											else if(name_buf[i] == '_')
											{
												user_tf = 1;
											}
											else
											{
												user_tf = 0;
												break;
											}
										}
									}
								}
								send(sd, &msgSizeOut, sizeof(uint16_t), 0);
								send(sd, &name_buf, msgSizeOut, 0);
							} 
							else if(recv_buf[0] == 'N')
							{
								//client cannot connect 
								printf("client cannot connect right now!\n");
								close(sd);
								exit(0);
							}
						} 
						else 
						{
							if(recv_buf[0] == 'Y')
							{
								//connect client
								hasUsrName = 1;
							} 
							else if (recv_buf[0] == 'N')
							{
								//client cannot connect 
								printf("client cannot connect right now!\n");
								close(sd);
								exit(0);
							}
						} 
					}
					else if(n == 0)
					{
						close(sd);
						exit(0);
					}
					else 
					{
						printf("'%s'\n" , recv_buf);
					}
					memset(&recv_buf[0], 0, sizeof(recv_buf));
				}
			}
		}
		memset(&send_buf[0], 0, sizeof(send_buf));
	}
	close(sd);

	exit(EXIT_SUCCESS);
}