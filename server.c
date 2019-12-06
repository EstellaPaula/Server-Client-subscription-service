#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helpers.h"
#include <math.h>

typedef struct value {
   char  data[1600];
   int length;
} value;

typedef struct topic {
   char  name[50];
   int nrValues;
   //struct value values[30];
} topic;

typedef struct abonament{
	char name[50];
	int nrToSend;
	struct value toSend[10];
	char sf[2];
} abonament;


typedef struct client {
	int socket;
	int on;
	struct abonament subs[10];
	int nrsubs;
	char id[10];
} client;



void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}


int main(int argc, char *argv[])
{
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr, my_sockaddr, from_station;
	int tcpsock, newsockfd, portno;
	int n, i, f, k, j, ret, ret2;
	int nrTopics = 0;		//numar "topics" in baza de date a serverului
	int primit = 0;	
	int nrClienti = 0;			//numar clienti
	int pos = 0;				//pozitie client cautat
	int pos2 = 0;
	unsigned int size = sizeof(struct sockaddr_in);
	socklen_t clilen;
	topic topics[100];			//vector cu toate "topics"			
	struct client clients[20];	//vector ce retine toti clientii
	char nume[50];
	struct value tempValue;

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds

	if (argc < 2) {
		usage(argv[0]);
	}

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	tcpsock = socket(AF_INET, SOCK_STREAM, 0);
	int udpsock = socket(PF_INET, SOCK_DGRAM, 0);
	DIE(tcpsock < 0, "socket");

	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");
	my_sockaddr.sin_family = AF_INET;
    my_sockaddr.sin_port = htons(8016);
    inet_aton("127.0.0.1", &my_sockaddr.sin_addr);

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	ret2 = bind(udpsock, (struct sockaddr *) &my_sockaddr, sizeof(struct sockaddr_in));

	ret = bind(tcpsock, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");
	DIE(ret2 < 0, "bind udp");

	ret = listen(tcpsock, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	// se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(tcpsock, &read_fds);
	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(udpsock, &read_fds);
	fdmax = tcpsock;
	if(udpsock > tcpsock) {
		fdmax = udpsock;
	}
	else {
		fdmax = tcpsock;
	}

	while (1) {
		tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == tcpsock) {
					// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
					clilen = sizeof(cli_addr);
					newsockfd = accept(tcpsock, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");

					// se adauga noul socket intors de accept() la multimea descriptorilor de citire
					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax) { 
						fdmax = newsockfd;
					}

				} else if (i == STDIN_FILENO) {
					memset(buffer, 0, BUFLEN);
					scanf("%s", buffer);
					//printf("I typed this: %s\n", buffer);
					if(buffer[0] == 'e' && buffer[1] == 'x' && buffer[2] == 'i' && buffer[3] == 't') {
						//printf("I stopped the server\n");
						n = send(i, buffer, strlen(buffer), 0);
						for (int k = 0; k <= fdmax; k++) {
							close(k);
						}
						return 0;
					} else {
						//printf("Not a valid command. Try again\n");
					}
				} else if (i == udpsock) {
					memset(buffer, 0, BUFLEN);
					memset(nume, 0, 50);
				    if((primit = recvfrom(udpsock, buffer, BUFLEN, 0, (struct sockaddr *) &from_station, &size)) >= 0) {
					    memcpy(nume, buffer, 50);
					    pos = -1;

					    //verificam daca topicul exista deja, si ii returnam pozitia din vector
					    for (j = 0; j < nrTopics; j++) {
					    	if (strcmp(topics[j].name, nume) == 0) {
					    		pos = j;
					    	}
					    }

					    if (pos > 0) {
					    //daca topicul exista deja
					    	topics[pos].nrValues++;
					    	char* adresaIP = inet_ntoa(from_station.sin_addr);	//adresa ip UDP
					    	char portaddr[5];
					    	sprintf(portaddr, "%d", ntohs(from_station.sin_port));	//port UDP
					    	memcpy(buffer + 1552, adresaIP, strlen(adresaIP));	//adauga ip in buffer
					    	memcpy(buffer + 1570, portaddr, strlen(portaddr));	//adauga port in buffer

					    	//---cautare clienti inscrisi la topicul primit---//
					    	for(j = 0; j < nrClienti; j++) {
					    		for(k = 0; k < clients[j].nrsubs; k++ ){
					    			if((strcmp(topics[pos].name, clients[j].subs[k].name) == 0) && (clients[j].on == 0)
					    				&& (strncmp(clients[j].subs[k].sf, "1",strlen("1")) == 0)) {
					    				int temp = clients[j].subs[k].nrToSend;
					    				memcpy(clients[j].subs[k].toSend[temp].data, buffer, BUFLEN);
					    				clients[j].subs[k].nrToSend++;
					    			}
					    			if((strcmp(topics[pos].name, clients[j].subs[k].name) == 0) && (clients[j].on == 1)) {
					    				n = send(clients[j].socket, buffer, BUFLEN, 0);
										DIE(n < 0, "send on server");
					    			}
					    		}
					    	}
					    } else {
					    	//daca topicul nu exita deja
						   	memcpy(topics[nrTopics].name, buffer, 50);
						   	tempValue.length = primit - 51;
						   	memcpy(tempValue.data, buffer + 51, tempValue.length);
					    	topics[nrTopics].nrValues = 1;
					    	nrTopics++;
					    	int curent = nrTopics -1;
		
					    	char* adresaIP = inet_ntoa(from_station.sin_addr);	//adresa ip UDP
					    	char portaddr[5];
					    	sprintf(portaddr, "%d", ntohs(from_station.sin_port));	//port UDP
					    	memcpy(buffer + 1552, adresaIP, strlen(adresaIP));	//adauga ip in buffer
					    	memcpy(buffer + 1570, portaddr, strlen(portaddr));	//adauga port in buffer

					    	//---cautare clienti inscrisi la topicul primit---//
					    	for(j = 0; j < nrClienti; j++) {
					    		for(k = 0; k < clients[j].nrsubs; k++ ){
					    			if((strcmp(topics[curent].name, clients[j].subs[k].name) == 0) && (clients[j].on == 0)
					    				&& (strncmp(clients[j].subs[k].sf, "1",strlen("1")) == 0)) {
					    				memcpy(clients[j].subs[k].toSend[0].data, buffer, BUFLEN);
					    				clients[j].subs[k].nrToSend = 1;
					    			}
					    			if((strcmp(topics[curent].name, clients[j].subs[k].name) == 0) && (clients[j].on == 1)) {
					    				n = send(clients[j].socket, buffer, BUFLEN, 0);
										DIE(n < 0, "send on server");
					    			}
					    		}
					    	}
					    }
					}
				} else {
					// s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");

					if (n == 0) {
						// conexiunea s-a inchis
						for(j = 0; j < nrClienti; j++) {
							if(clients[j].socket == i){
								pos = j;
								break;
							}
						}
						clients[pos].socket = -1;
						clients[pos].on = 0;
						printf("Client %s disconnected .\n",clients[pos].id );
						close(i);
						
						// se scoate din multimea de citire socketul inchis 
						FD_CLR(i, &read_fds);
					} else {	

						char del[] = " ";
						char* token;
						token = strtok(buffer, del);
						char subscribe[50];
						sprintf(subscribe, "%s", token);

						if(strcmp(subscribe, "subscribe") == 0) {
							char topic[50];
							token = strtok(NULL, del);
							sprintf(topic, "%s", token);
							char id[10];
							char sf[2];
							token = strtok(NULL, del);
							memcpy(sf,token,2);
							token = strtok(NULL, del);
							sprintf(id, "%s", token);

							if (!((strncmp(sf, "1",strlen("1")) == 0) || (strncmp(sf, "0",strlen("0")) == 0))) {
								memset(buffer, 0, BUFLEN);
								memcpy(buffer, "error3",6);
								n = send(i, buffer, BUFLEN, 0);
								DIE(n < 0, "send on server");
							} else {
								pos = -1;
								for(j = 0; j < nrTopics; j++) {
									if (strncmp(topic, topics[j].name, strlen(topics[j].name)) == 0) {
										pos = 1;
									}
								}

								if (pos < 0) {
									memset(buffer, 0, BUFLEN);
									memcpy(buffer, "error1",6);
									n = send(i, buffer, BUFLEN, 0);
									DIE(n < 0, "send on server");
								}

								pos = -1;
								for(j = 0; j < nrClienti; j++) {
									if(strcmp(clients[j].id, id) == 0){
										pos = j;
										break;
									}
								}
								int nrSubs = clients[pos].nrsubs;
								sprintf(clients[pos].subs[nrSubs].name, "%s" ,topic);
								sprintf(clients[pos].subs[nrSubs].sf, "%s", sf);
								clients[pos].subs[nrSubs].nrToSend = 0;
								clients[pos].nrsubs++;
							}
						} else if (strcmp(subscribe, "unsubscribe") == 0) {
							char topic[50];
							token = strtok(NULL, del);
							sprintf(topic, "%s", token);
							char id[10];
							token = strtok(NULL, del);
							sprintf(id, "%s", token);

							pos = -1;
							for(j = 0; j < nrClienti; j++) {
								if(strcmp(clients[j].id, id) == 0){
									pos = j;
									break;
								}
							}

							pos2 = -1;
							for (k = 0; k < clients[pos].nrsubs; k++) {
								if (strncmp(topic, clients[pos].subs[k].name, strlen(clients[pos].subs[k].name)) == 0) {
									sprintf(clients[pos].subs[k].name, "-UNSUBSCRIBED-");
									pos2 = 1;
									break;
								}
							}
							if (pos2 < 0) {
								memset(buffer, 0, BUFLEN);
								memcpy(buffer, "error2",6);
								n = send(i, buffer, BUFLEN, 0);
								DIE(n < 0, "send on server");
							}

						} else {

							printf("New client %s connected from %s:%d\n", 
								subscribe, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

							//----check if client is new---//
							pos = -1;
							pos2 = -1;
							for (j = 0; j < nrClienti; j++) {
								if (strcmp(clients[j].id, subscribe) == 0) {
									pos = j;
									if (clients[j].on == 1) {
										pos2 = 1;
									}
								}
							}
							if(pos2 == 1) {
								memset(buffer, 0, BUFLEN);
								memcpy(buffer, "error4",6);
								n = send(i, buffer, BUFLEN, 0);
								DIE(n < 0, "send on server");
							} else if(pos < 0) {			//client cu id nou ce trebuie adaugat in vector
								strcpy(clients[nrClienti].id, subscribe);
								clients[nrClienti].on = 1;
								clients[nrClienti].nrsubs = 0;
								clients[nrClienti].socket = i;
								nrClienti++;

							} else {
								clients[pos].on = 1;
								clients[pos].socket = i;

								for(k = 0; k < clients[pos].nrsubs; k++) {
									if ((clients[pos].subs[k].nrToSend != 0) && (clients[pos].on == 1)) {
										int snd = clients[pos].subs[k].nrToSend;
										for(f = 0; f < snd; f++) {
											n = send(clients[pos].socket, clients[pos].subs[k].toSend[f].data, BUFLEN, 0);
											DIE(n < 0, "send on server");
										}
										clients[pos].subs[k].nrToSend = 0;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	close(tcpsock);

	return 0;
}
