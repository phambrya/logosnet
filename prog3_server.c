/*
Bryan Pham
Project 3: LogosNet
Due: July 31, 2021
Description: This is a server file that will be the server for a two distinct types of 
client for a simple online chat application. The server should be able to handle up to
255 participants and an equal amount of observers.
*/

#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <unistd.h>

#define QLEN 6 /* size of request queue */
int visits = 0; /* counts client connections */

/* 
	Use a link list from this website,
	https://www.tutorialspoint.com/data_structures_algorithms/linked_list_program_in_c.htm
 */

struct node {
   int key;
   int hasName;
   int type;
   int observerPort;
   char userName[12];
   struct node *next;
};

struct node *head = NULL;
struct node *current = NULL;

//display the list
void printList() {
   struct node *ptr = head;
   printf("\n[ ");
	
   //start from the beginning
   while(ptr != NULL) {

	printf("(key[%d],hasName[%d], type[%d] usrName[%s]) \n",ptr->key,ptr->hasName, ptr->type, ptr->userName);

      ptr = ptr->next;
   }
	
   printf(" ]\n");
}

//insert link at the first location
void insertFirst(int key, int hasName, int type, int observerPort, char username[12]) {
   //create a link
   struct node *link = (struct node*) malloc(sizeof(struct node));
	
   link->key = key;
   link->hasName = hasName;
   link->type = type;
   link->observerPort = observerPort;
   strcpy(link->userName, username);

   //point it to old first node
   link->next = head;
	
   //point first to new first node
   head = link;
}

//find a link with given key
struct node* find(int key) {

   //start from the first link
   struct node* current = head;

   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(current->key != key) {
	
    //if it is last node
    if(current->next == NULL) {
        return NULL;
    } 

    //go to next link
    current = current->next;
   }      
	
   //if data found, return the current Link
   return current;
}

//delete a link with given key
struct node* delete(int key) {

   //start from the first link
   struct node* current = head;
   struct node* previous = NULL;
	
   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(current->key != key) {

	//if it is last node
	if(current->next == NULL) {
		return NULL;
	} 

	//store reference to current link
	previous = current;
	//move to next link
	current = current->next;

   }

   //found a match, update the link
   if(current == head) {
      //change first to point to next link
      head = head->next;
   } else {
      //bypass the current link
      previous->next = current->next;
   }    
	
   return current;
}


int main(int argc, char **argv) 
{
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold server's address */
	struct sockaddr_in sad2;
	struct sockaddr_in cad; /* structure to hold client's address */
	int sd, sd2, sd3; /* socket descriptors */
	int port, port2; /* protocol port number */
	int alen; /* length of address */
	int optval = 1; /* boolean value when we set socket option */
	char buf[1000]; /* buffer for string the server sends */

	if( argc != 3 ) 
	{
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server server_port\n");
		exit(EXIT_FAILURE);
	}

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */

	memset((char *)&sad2,0,sizeof(sad2)); /* clear sockaddr structure */


	sad.sin_family = AF_INET;
	
	sad.sin_addr.s_addr = INADDR_ANY;
   	/* convert argument to binary */ 
	port = atoi(argv[1]);
	if (port > 0) 
	{ 
		sad.sin_port = htons(port);
	} 
	else 
	{ 
		fprintf(stderr,"Error: Bad port number %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}

	sad2.sin_family = AF_INET;
	
	sad2.sin_addr.s_addr = INADDR_ANY;
    /* convert argument to binary */
	port2 = atoi(argv[2]); 
	if (port2 > 0) 
	{ 
		sad2.sin_port = htons(port2);
	} 
	else 
	{ 
		fprintf(stderr,"Error: Bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}

	/* Map TCP transport protocol name to protocol number */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	sd = socket(AF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}


	sd2 = socket(AF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd2 < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Allow reuse of port - avoid "Bind failed" issues */
	if( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		exit(EXIT_FAILURE);
	}

	if( setsockopt(sd2, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		exit(EXIT_FAILURE);
	}

	if (bind(sd, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}

	if (bind(sd2, (struct sockaddr*) &sad2, sizeof(sad2)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}

	if (listen(sd, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}

	if (listen(sd2, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}


	fd_set master;		//master list
	fd_set read_fds; 	//temp file file descriptor list
	int fdmax;
	
	
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	//add sd to master set
	FD_SET(sd, &master);
	FD_SET(sd2, &master);

	fdmax = sd;

	uint16_t msgSize = UINT16_MAX;

	/* Main server loop - accept and handle requests */
	while (1) {
		read_fds = master; //copy master
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
			perror("select");
			exit(4);
		}
		//running through existing connections 
		for (int i = 0; i <= fdmax; i++)
		{ 
			if (FD_ISSET(i, &read_fds))
			{
				FD_CLR(i, &read_fds);
				// Handling new connections
				if (i == sd)
				{
					alen = sizeof(cad);
					if ((sd3 = accept(sd, (struct sockaddr *)&cad, &alen)) < 0) 
					{
						fprintf(stderr, "Error: Accept failed\n");
						exit(EXIT_FAILURE);
					}
					//add cleared connection to set
					FD_SET(sd3, &master); // add to master set
					if (sd3 > fdmax) 
					{
						fdmax = sd3;
						char response = 'Y';
						send(sd3, &response, sizeof(char), 0);
						char new[12];
						strncpy(new, "New", 3);
						insertFirst(sd3, 0, 1, 0, new);
					} 
					else 
					{
						char response = 'N';
						send(sd3, &response, sizeof(char), 0);
						printf("denied new client in set! index: %d\n", sd3);
					}
				} 
				else if (i == sd2) 
				{
					alen = sizeof(cad);
					if ((sd3 = accept(sd2, (struct sockaddr *)&cad, &alen)) < 0) {
						fprintf(stderr, "Error: Accept failed\n");
						exit(EXIT_FAILURE);
					}
					FD_SET(sd3, &master); // add to master set
					if (sd3 > fdmax) 
					{
						fdmax = sd3;
						char response = 'Y';
						send(sd3, &response, sizeof(char), 0);
						char new[12];
						strncpy(new, "New", 3);
						insertFirst(sd3, 0, 2, 0, new);
					} 
					else 
					{
						char response = 'N';
						send(sd3, &response, sizeof(char), 0);
						printf("denied new observer in set! index: %d\n", sd3);
					}
				} 
				//Receiving data from clients
				else 
				{
					struct node *foundLink = find(i);
					if(foundLink != NULL) {
						if(foundLink->type == 1){

							char recv_buf[1000];
							char userName[1014];
							int recv_n;

							if(msgSize == UINT16_MAX)
							{
								uint8_t check;
								recv(i, &check, 1, 0);
								//printf("CHECK NUM: %d\n", check);
								if(check == 16)
								{
									recv_n = recv(i, &msgSize, sizeof(uint16_t), 0);
								}
								else if(check == 8)
								{
									recv_n = recv(i, &msgSize, 1, 0);	
								}
								else if(check == 0)
								{
									delete(foundLink->key);
									//printList();

								}

								if(recv_n <= 0){

									char send_buf[1000];
									memset(&send_buf[0], 0, sizeof(send_buf));	
									FD_CLR(i, &master);
									strncpy(send_buf, "User ", 5);
									strncat(send_buf, foundLink->userName, strlen(foundLink->userName));
									strncat(send_buf, " has left chat", 14);
									//remove the user from linked list
									char userNameCheck[12];
									strncpy(userNameCheck, foundLink->userName, strlen(foundLink->userName));
									strncpy(userName, foundLink->userName, strlen(foundLink->userName));
									//printList();
									delete(foundLink->key);

									for(int j=0; j <= fdmax; j++){
										if (FD_ISSET(j, &master)){
											if (j != sd) {
												struct node *foundLink = find(j);
												if(foundLink != NULL) {
													if(foundLink->type == 2){
														if(send(j, send_buf, strlen(send_buf), 0) == -1) {
															perror("send");
														}
														if(strcmp(foundLink->userName, userNameCheck) == 0)
														{
															//printf("disconnect");
															delete(foundLink->key);
															//printList();
														}
													}
												}		
											}
										}
									}//end of broadcast
									memset(&send_buf[0], 0, sizeof(send_buf));
								}
							} 
							else {
								memset(&recv_buf[0], 0, sizeof(recv_buf));
								//printf("MSGSize: %d\n", msgSize);
								recv(i, &recv_buf, msgSize, 0);
								//printf("BUFF: ---%s---\n", recv_buf);
								if(foundLink->hasName == 0)
								{	
									//see if username is taken
									char response = 'Y';
									send(i, &response, sizeof(char), 0);
									memset(&response, 0, sizeof(response));
									foundLink->hasName = 1;

									char name[12];
									memset(&name[0], 0, sizeof(name));	
									strncpy(name, recv_buf, strlen(recv_buf));
									//strncpy(foundLink->userName, recv_buf, strlen(recv_buf));
									//printf("NAME: %s\nLENGTH: %lu\n", name, strlen(name));
									memset(&foundLink->userName, 0, sizeof(foundLink->userName));
									strncpy(foundLink->userName, name, strlen(name));
									//printf("userName: %s\n",foundLink->userName);
									// *** broadcast username *** //
									char send_buf[1000];
									memset(&send_buf[0], 0, sizeof(send_buf));
									strncpy(send_buf, "User ", 5);
									strncat(send_buf, foundLink->userName, strlen(foundLink->userName));
									strncat(send_buf, " has entered chat", 17);

									for(int j=0; j <= fdmax; j++)
									{
										if (FD_ISSET(j, &master))
										{
											if (j != sd) 
											{
												struct node *foundLink = find(j);
												if(foundLink != NULL) {
													if(foundLink->type == 2)
													{
														//send message size 

														//send message
														if (send(j, send_buf, strlen(send_buf), 0) == -1) 
														{
															perror("send");
														}
													}
												}		
											}
										}
									}//end of broadcast
									memset(&response, 0, sizeof(response));
									memset(&send_buf[0], 0, sizeof(send_buf));
								}
								else if(recv_buf[0] == '@')
								{
									//printList();
									char sender[12];
									memset(&sender[0], 0, sizeof(sender));
									strncpy(sender, foundLink->userName, strlen(foundLink->userName));
									int sender_Port = foundLink->observerPort;
									char *space_ptr = strchr(recv_buf, ' ');
									int posn = -1;
									if (space_ptr != NULL)
									{
										posn = space_ptr - recv_buf;
										char buffer[posn-1];
										memcpy(buffer, &recv_buf[1], posn-1);
										int match;
										for(int j = 0; j <= fdmax; j++){
											if(j != i){
												struct node *foundLink = find(j);
												if(foundLink != NULL) 
												{
													if(foundLink->type == 1)
													{
														match = 0;
														for(int k=0; k < sizeof(buffer); k++)
														{
															if(foundLink->userName[k] == buffer[k])
															{
																match = 1;
															} 
															else 
															{
																match = 0;
																break;
															}	
														}
														if(match == 1)
														{
															char* substr = recv_buf + posn;
															memmove(recv_buf, substr, strlen(substr) + posn);
															strncpy(userName, "-", 1);
															strncat(userName, sender, strlen(sender));
															strncat(userName, " :", 2);
															strncat(userName, recv_buf, strlen(recv_buf));
															if (send(foundLink->observerPort, userName, strlen(userName), 0) == -1) {
																perror("send");
															}
															if (send(sender_Port, userName, strlen(userName), 0) == -1) {
																perror("send");
															}
														}
													}
												}
											}
										}
									} 
								} 
								else {

									// *** broadcast to all *** //

										char send_buf[1000];
										memset(&send_buf[0], 0, sizeof(send_buf));
										strncpy(send_buf, ">", 1);
										strncat(send_buf, foundLink->userName, strlen(foundLink->userName));
										strncat(send_buf, " :", 2);
										strncat(send_buf, recv_buf, strlen(recv_buf));
										//printf("BUFFF: %s\n", recv_buf);
										for(int j = 0; j <= fdmax; j++){
											if (FD_ISSET(j, &master)){
												if (j != sd) {
													struct node *foundLink = find(j);
													if(foundLink != NULL) 
													{
														if(foundLink->type == 2)
														{
															//send message
															if (send(j, send_buf, strlen(send_buf), 0) == -1) 
															{
																perror("send");
															}
														}
													}
												}
											}
										}//end of broadcast
										//printf("Buff1: %s\n", recv_buf);
										memset(&send_buf[0], 0, sizeof(send_buf));
									
								}
								memset(&recv_buf[0], 0, sizeof(recv_buf));
								memset(&userName[0], 0, sizeof(userName));
								msgSize = UINT16_MAX;
							}
						}
						// Observer
						else if(foundLink->type == 2)
						{

							char recv_buf[1000];
							char userName[1014];
							int recv_n;
							
							if(msgSize == UINT16_MAX)
							{
								recv_n = recv(i, &msgSize, sizeof(uint16_t), 0);

								//printf("PRINT: %d\n", msgSize);
								if(msgSize == 0)
								{
									msgSize = UINT16_MAX;
									delete(foundLink->key);
									//printList();
								}
								if(recv_n <= 0)
								{
									FD_CLR(i, &master);
									char send_buf[1000];

									FD_CLR(i, &master);
									memset(&send_buf[0], 0, sizeof(send_buf));
									strncpy(send_buf, "Observer has left the chat", 26);
									
									for(int j=0; j <= fdmax; j++)
									{
										if (j != sd && j != i) {
											if (FD_ISSET(j, &master))
											{
												//send message size 

												if (send(j, send_buf, strlen(send_buf), 0) == -1) 
												{
													delete(foundLink->key);
													perror("send");
												}
											}
										}
									}//end of broadcast
									memset(&send_buf[0], 0, sizeof(send_buf));
								}
							} 
							
							else 
							{
								memset(&recv_buf[0], 0, sizeof(recv_buf));	
								recv_n = recv(i, &recv_buf, msgSize, 0);
								if(msgSize == 0)
								{
									delete(foundLink->key);
									//printList();
								}
								if(recv_n <= 0)
								{
									FD_CLR(i, &master);
									char send_buf[1000];

									FD_CLR(i, &master);
									memset(&send_buf[0], 0, sizeof(send_buf));	
									strncpy(send_buf, "Observer has left the chat", 26);
									
									for(int j=0; j <= fdmax; j++)
									{
										if (j != sd && j != i) 
										{
											if (FD_ISSET(j, &master))
											{
												if (send(j, send_buf, strlen(send_buf), 0) == -1) 
												{
													perror("send");
												}
											}
										}
									}//end of broadcast
									memset(&send_buf[0], 0, sizeof(send_buf));
									
								} 
								else if(foundLink->hasName == 0)
								{
									//see if username is taken or available 
									char response = 'N';
									for(int j = 0; j <= fdmax; j++)
									{
										struct node *foundLink = find(j);
										if(foundLink != NULL) 
										{
											if(foundLink->type == 1)
											{
												int match = 0;
												if(strlen(foundLink->userName) == strlen(recv_buf))
												{
													for(int k = 0; k < strlen(recv_buf); k++)
													{
														if(foundLink->userName[k] == recv_buf[k])
														{
															match = 1;
														} 
														else 
														{
															match = 0;
															break;
														}	
													}
												}
												if(match == 1)
												{
													foundLink->observerPort = i;
													response = 'Y';
													j = fdmax;
												}
											}
										}
									}
									send(i, &response, sizeof(char), 0);
									memset(&response, 0, sizeof(response));
									foundLink->hasName = 1;
									memset(&foundLink->userName, 0, sizeof(foundLink->userName));	
									strncpy(foundLink->userName, recv_buf, strlen(recv_buf));

									// *** broadcast username *** //
									char send_buf[1000];
									memset(&send_buf[0], 0, sizeof(send_buf));
									strncpy(send_buf, "Observer has entered chat", 25);

									for(int j=0; j <= fdmax; j++)
									{
										if (FD_ISSET(j, &master))
										{
											if (j != sd) {
												struct node *foundLink = find(j);
												if(foundLink != NULL) 
												{
													if(foundLink->type == 2)
													{
														if (send(j, send_buf, strlen(send_buf), 0) == -1) 
														{
															perror("send");
														}
													}
												}
											}
										}
									}
									//printList();//end of broadcast
									memset(&send_buf[0], 0, sizeof(send_buf));
									memset(&response, 0, sizeof(response));
								}
								memset(&recv_buf[0], 0, sizeof(recv_buf));
								memset(&userName[0], 0, sizeof(userName));
								msgSize = UINT16_MAX;
							}
						} 
					}
				}
			}
		}
	}
	close(sd2);
}
