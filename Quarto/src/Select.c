#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/shm.h>
#include "SharedVariables.h"
#include "Errmmry.h"
#include "Playtime.h"
#include "PerformConnection.h"

#define BUFFR 512

/**
 * Liest den Zug, den der Thinker berechnet hat aus der Pipe aus
 * und sendet diesen an den Server
 *
 * @param Socket, Buffer,pipe
 * @return 0 falls move ok
 */
int doMove(int sock, char* buffer, int fd[]) {
	int size;
	char* reply = malloc(sizeof(char) * 15);
	addchar(reply);
	size = read(fd[0], reply, 15);
	if (size < 0) {
		perror("Fehler beim Lesen der Pipe");
		return EXIT_FAILURE;
	}

	sendReplyFormatted(sock, reply);
	size = recv(sock, buffer, BUFFR - 1, 0);
	if (size > 0) {
		buffer[size] = '\0';
	}
	if (strcmp(buffer, "+ MOVEOK\n") == 0) {
		printf("\nZug wurde akzeptiert!\n");
	} else {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * Diese Funktion implementiert select() und wechselt zwischen der Berechnung des naechsten
 * Zugs (Vater-Thinker) und der fortfuehrung der Kommunikation mit dem Server (Kind-Connector)
 *
 * @param Socket fuer Kommunikation mit Server, Buffer fuer auslesen der Antwort, SHM
 * @return 0
 */
int waitforfds(int sock, char* buffer, sharedmem * shm, int fd[]) {
	int rc, size, status, biggest;
	int pipe = fd[0];
	fd_set fds;
	struct timeval timeout;

	if (pipe > sock) {
		biggest = pipe;
	} else {
		biggest = sock;
	}

	status = checkServerReply(sock, buffer, shm);
    if (status != 0) {
        return EXIT_FAILURE;
    }

	do {
		/* Unsere 2 Filedescriptoren werden hinzugefuegt. (Socket und Pipe) */
		FD_ZERO(&fds);
		FD_SET(sock, &fds);
		FD_SET(pipe, &fds);

		/* Zeitlimit fuer select wird eingestellt (hier 10 Sekunden, kann beliebig angepasst werden!) */
		timeout.tv_sec = 30;
		timeout.tv_usec = 0;

		/* Select laesst den Prozess schlafen und beobachtet fuer eine in timeout spezifizierte Zeit lang die in fds hinzugefuegten
		 * Filedescriptoren (in unserem Falle Socket und Pipe).
		 * Falls waehrenddessen ein Filedescriptor ready to read wird, weckt select den Prozess auf und gibt die Anzahl der ready to read
		 * Filedescriptoren zurueck */
		rc = select(biggest + 1, &fds, NULL, NULL, &timeout);
		if (rc == -1) {
			perror("Error, select failed! \n");
			return EXIT_FAILURE;
		}

		if (rc > 0) {
			/* Fall der Socket ready to read ist */
			if (FD_ISSET(sock, &fds)) {
				size = recv(sock, buffer, BUFFR - 1, 0);
				writelog(logdatei, AT);
				if (size > 0) {
					buffer[size] = '\0';
				}
				status = checkServerReply(sock, buffer, shm);
				if (status != 0) {
					return EXIT_FAILURE;
				}

			/* Fall die Pipe ready to read ist */
			} else if (FD_ISSET(pipe, &fds)) {
				doMove(sock, buffer, fd);
<<<<<<< HEAD
=======
               
>>>>>>> e9e727f01912596c27bd88666bc4cbfc0439a567
			}

		/* Falls select 0 returned bedeutet dies, dass die Wartezeit ohne Ereignis abgelaufen ist.*/
		} else if (rc == 0) {
			perror("Select timed out!. \n");
		}
	}
	while (rc != 0);

	return EXIT_SUCCESS;
}

