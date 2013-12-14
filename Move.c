#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/shm.h>
#include "sharedVariables.h"
#include "errmmry.h"
#include "auxiliaryFunctions.h"

#define BUFFR 512

int doMove(int sock, char* buffer, sharedmem * shm) {
	int size;
	char movebfr[BUFFR];
	sendReplyFormatted(sock, "THINKING");
	size = recv(sock, buffer, BUFFR - 1, 0);
	if (size > 0)
		buffer[size] = '\0';
	printf("%s", buffer);
	if (strcmp(buffer, "+ OKTHINK\n") != 0)
		size = recv(sock, buffer, BUFFR - 1, 0);
	printf("\n Dein Zug: ");
	fgets(movebfr, BUFFR, stdin);
	char* temp;
	temp = antistrcat(movebfr, "PLAY ");
	sendReplyFormatted(sock, temp);
	printf("%s", temp);
	size = recv(sock, buffer, BUFFR - 1, 0);
	if (size > 0)
		buffer[size] = '\0';
	printf("%s", buffer);
	if (strcmp(buffer, "+ MOVEOK\n") == 0) {
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}
