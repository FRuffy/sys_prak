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
