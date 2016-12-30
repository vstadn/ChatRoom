/* Vitalii Stadnyk
 * CHAT PROJECT
 * CS 410 
   Internet domain TCP server. 
   Usage: ./server <port> 
   Really only useful with client 
   Original code from Linux HowTos.
*/

#include "c-s-socket.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

// this function is used by every new thread in order to be able to read anything
// sent from other users and to send what this particular thread wants to send to 
// other users

void* execute(void* sockedfd);

int users[10];
char user_names[10][256];
char temporary[256];

int main(int argc, char* argv[]){
    int i;

    for (i = 0; i < 10; i++) {
        strcpy(temporary, "Not authenticated user: ");
        strcpy(user_names[i], temporary);
    }

    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
        fprintf(stderr, "usage %s port\n", argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char*)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    while (newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen)) {
        pthread_t thread_x;
        pthread_create(&thread_x, NULL, execute, &newsockfd);
    }
    close(newsockfd);
    close(sockfd);

    return (0);
}

void* execute(void* newsocketfd){

    int connected = 1;
    char message[256];
    char username[256];
    char storage[256];
    char quit[256];
    char post[256];
    char who[256]; 
    char help[256];
    int n;
    int sockfd = *(int*)newsocketfd;
    int i;
    char temporary[256];
    users[sockfd] = 1;

    strcpy(quit, "/QUIT");
    strcpy(username, "/USER");
    strcpy(post, "/POST");
    strcpy(who, "/WHO");
    strcpy(help, "/HELP");

    for (i = 0; i < 10; i++) {
        if (users[i] == 1) {
            strcpy(message, "New user has joined");
            n = write(i, message, strlen(message));
            strcpy(message, "");
        }
    }

    while (connected == 1) {
        bzero(storage, 256);
        n = read(sockfd, storage, 255);
        if (n < 0)
            error("ERROR reading from socket");

        if (strncmp(username, storage, strlen(username)) == 0) {
            memset(user_names[sockfd], 0, strlen(user_names[sockfd]));
            strncpy(user_names[sockfd], storage + 6, strlen(storage) - 6);
        }
	
	// POST option
        else if (strncmp(post, storage, strlen(username)) == 0) {
            for (i = 0; i < 10; i++) {
                if (users[i] == 1) {
                    strcpy(message, "");
                    strcat(message, user_names[sockfd]);
                    strcat(message, ": ");
                    strcat(message, storage + 6);
                    n = write(i, message, strlen(message));
                }
            }
        }
	// WHO option
        else if (strncmp(who, storage, strlen(username)) == 0) {
            strcpy(message, "");
            strcat(message, "Logged in Users are:\n");
            for (i = 0; i < 10; i++) {
                if (users[i] == 1) {
                    strcat(message, user_names[i]);
		    strcat(message,"\n");
		}
	    }
            n = write(sockfd, message, strlen(message));
        }

	// HELP option
        else if (strncmp(help, storage, strlen(username)) == 0) {
            strcpy(message, "");
            strcat(message, "Protocols are:\n");
            strcat(message, "'/USER SP Name CRLF'; login with the given name\n");
            strcat(message, "'/POST SP Message CRLF'; post message to all users\n");
            strcat(message, "'/WHO CRLF'; get list of logged in users\n");
            strcat(message, "'/HELP CRLF'; show list of protocol operations\n");
            strcat(message, "'/QUIT CRLF'; disconnect from the server\n");
            n = write(sockfd, message, strlen(message));
        }

	// QUIT option
        else if (strncmp(quit, storage, strlen(quit)) == 0) {
	    //setting username to non authenticated when one disconnects
            strcpy(temporary, "Unknown User");
            strcpy(user_names[sockfd], temporary);
	    printf("closing connection over sockfd: %d \n", sockfd);
	    users[sockfd] = 0;
            connected = 0;
            strcpy(message, "/QUIT");
            n = write(sockfd, message, strlen(message));
        }
    }
    close(sockfd);
}
