#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <math.h>
#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_address server_port\n", file);
	exit(0);
}


void decrypt(int type, char  data[1500], char port[20], char ip[20], char topic[50]) {
	float p;
	printf("%s:%s - %s -", ip, port, topic);
	if(type == 0) {
		uint32_t dataint;
		char semn = 0;
		memcpy(&dataint, data + 1, sizeof(uint32_t));
		memcpy(&semn, data, 1);
		dataint = ntohl(dataint);
		printf(" INT - ");
		if(semn == 1) printf("-");
		printf("%u\n", dataint);
		} else if(type == 1) {
			printf(" SHORT_REAL - ");
			uint16_t datauint;
			memcpy(&datauint, data, sizeof(uint16_t));
			datauint = ntohs(datauint);
			printf("%.2f\n", (double)datauint / 100);
		} else if(type == 2) {
			printf(" FLOAT - ");
			uint32_t datafloat;
			char semn = 0;
			uint8_t putere;
			memcpy(&datafloat, data + 1, sizeof(uint32_t));
			memcpy(&semn, data, 1);				    		
			memcpy(&putere, data + 1 + sizeof(uint32_t), sizeof(uint8_t));
		    datafloat = ntohl(datafloat);
			if(semn == 1) printf("-");
			p = 1 / pow(10, putere);
			p = datafloat * p;
			printf("%.4f\n", p);
		} else if(type == 3) {
			printf(" STRING - ");
			printf("%s\n", data);
		}
}


int main(int argc, char *argv[])
{
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];
	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax, ret2;
	char data[1500];
	int type = 0;
	char name[50];
	int length = 0;
	int count = 0;
	int k = 0;

	if (argc < 4) {
		usage(argv[0]);
	}
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");
	FD_SET(sockfd, &read_fds);
	FD_SET(STDIN_FILENO, &read_fds);
	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");
	fdmax = sockfd;

	char* id = argv[1];
	printf("\n");
	n = send(sockfd, id, strlen(id), 0);
	DIE(n < 0, "send");

	while (1) {
		tmp_fds = read_fds; 
		int ret2 = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret2 < 0, "select");
		for (int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd) {
					memset(buffer, 0, BUFLEN);
					n = recv(sockfd, buffer, sizeof(buffer), 0);
					if (n == 0) {
						close(sockfd);
						close(STDIN_FILENO);
						return 0;
					}
					char portaddr[20];
					char adresaIP[20];

					memcpy(adresaIP, buffer + 1552, 20);
				    memcpy(portaddr, buffer + 1570, 20);

				    memcpy(name, buffer, 50);
				    type = *(buffer + 50);
				    length = n - 151;
				    memcpy(data, buffer + 51, length);

				    if(buffer[0] == 'e' && buffer[1] == 'x' && buffer[2] == 'i' && buffer[3] == 't') {
						close(sockfd);
						close(STDIN_FILENO);
						return 0;
					} else if(buffer[0] == 'e' && buffer[1] == 'r' && buffer[2] == 'r' && buffer[3] == 'o'
						 && buffer[4] == 'r' && buffer[5] == '1') {
						//printf("There is no such topic. Subscription registered in case of further added topics.\n");
					} else if(buffer[0] == 'e' && buffer[1] == 'r' && buffer[2] == 'r' && buffer[3] == 'o'
						 && buffer[4] == 'r' && buffer[5] == '2') {
						//printf("Client not subscribed to this topic.\n");
					} else if(buffer[0] == 'e' && buffer[1] == 'r' && buffer[2] == 'r' && buffer[3] == 'o'
						 && buffer[4] == 'r' && buffer[5] == '3') {
						//printf("SF not valid. SF must be 1 or 0.\n");
					} else if(buffer[0] == 'e' && buffer[1] == 'r' && buffer[2] == 'r' && buffer[3] == 'o'
						 && buffer[4] == 'r' && buffer[5] == '4') {
						//printf("There is already a client with that id.Try another id\n");
						close(sockfd);
						close(STDIN_FILENO);
						return 0;
					} else {
				   		decrypt(type, data, portaddr, adresaIP, name);
				   	}
				} else {
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN, stdin);
					// se trimite mesaj la server
					if(strncmp(buffer, "subscribe", 9) == 0) {
						
						count = 1;
						for (k = 0; buffer[k] != '\0'; k++) {
							if (buffer[k] == ' ') {
								count++;
							}
						}

						if (count < 3) {
							//printf("Not enough arguments.\n");
						} else {
							strcat(buffer, " ");
							strcat(buffer, id);
							n = send(sockfd, buffer, strlen(buffer), 0);
							DIE(n < 0, "send");
							char* topic = strtok(buffer, " ");
							topic = strtok(NULL, " ");
							printf("subscribed to %s\n", topic);
						}
					} else if(strncmp(buffer, "unsubscribe", 11) == 0) {
						count = 1;
						for (k = 0; buffer[k] != '\0'; k++) {
							if (buffer[k] == ' ') {
								count++;
							}
						}

						if (count < 2) {
							//printf("Not enough arguments.\n");
						} else {
							strcat(buffer, " ");
							strcat(buffer, id);
							n = send(sockfd, buffer, strlen(buffer), 0);
							char* topic = strtok(buffer, " ");
							topic = strtok(NULL, " ");
							printf("unsubscribed to %s\n", topic);
							DIE(n < 0, "send");
						}
					} else if (strncmp(buffer, "exit", 4) == 0) {
						close(sockfd);
						close(STDIN_FILENO);
						return 0;
					} else {
						//printf("Not a valid command. Try another command than \n");
					}
				}
			}
		}

	}

	ret2 = 1;
	if(ret2 != 1) {
		printf("here ends\n");
	}

	close(sockfd);

	return 0;
}
