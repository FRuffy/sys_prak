#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/shm.h>
#include "sharedVariables.h"
#include "errmmry.h"
#include "auxiliaryFunctions.h"

#define BUFFR 512

int doMove(int sock, char* buffer,sharedmem * shm) {
	int size;

	sendReplyFormatted(sock, "THINKING");
	size = recv(sock, buffer, BUFFR - 1, 0);
	if (size > 0)
		buffer[size] = '\0';
	if (strcmp(buffer, "+ OKTHINK\n") != 0)
		size = recv(sock, buffer, BUFFR - 1, 0);

	// Naechsten Spielzug ausdenken (ueber thinker)
	shm->thinking = 1;
	shm->pleaseThink = 1;
	// Sagt dem Vater, dass er jetzt denken soll
	kill(shm->pidDad, SIGUSR1);
	sleep(1);
	while (shm->thinking == 1) { sleep(0.1); } //Warten bis fertiggedacht wurde

	char* reply = malloc(sizeof(char)*15);
	read(fd[0], reply, 15);
	printf("\n%s\n", reply);
	sendReplyFormatted(sock, reply);
	free(reply);


	size = recv(sock, buffer, BUFFR - 1, 0);
	if (size > 0)
		buffer[size] = '\0';
	if (strcmp(buffer, "+ MOVEOK\n") == 0) {
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

void loop(int sock, char* buffer, sharedmem * shm) {
	int status = checkServerReply(sock, buffer, shm);
	int moveStatus;
	int size;
	while (status == EXIT_SUCCESS) {
		moveStatus = doMove(sock, buffer, shm);
		size = recv(sock, buffer, BUFFR - 1, 0);
		if (size > 0)
			buffer[size] = '\0';
		if (moveStatus != EXIT_FAILURE) {
			status = checkServerReply(sock, buffer, shm);
		} else {
			break;
		}
	}
}

