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
 * Ueberprueft ob Zug ok und fuerht aus
 *
 * @param Socket, Buffer
 * @return 0 falls move ok
 */
int doMove(int sock, char* buffer, int fd[]) {
	int size;
	char* reply = malloc(sizeof(char) * 15);
	addchar(reply);
	size = read(fd[0], reply, 15);

	if (size < 0) {
		perror("Fehler bei read");
	}

	printf("\nReplyDoMove: %s\n", reply);
	sendReplyFormatted(sock, reply);
	size = recv(sock, buffer, BUFFR - 1, 0);

	if (size > 0) {
		buffer[size] = '\0';
	}

	if (strcmp(buffer, "+ MOVEOK\n") == 0) {
		printf("Zug wurde akzeptiert!\n");
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

	/* checkServerReply muss leider einmal ausserhalb der Schleife ausgefuehrt werden, da die Antwort des Servers schon frueher im Code
	 * vom Socket gelesen wurde und hier im buffer uebergeben wurde;
	 * Solange es zu keinem Timeout kommt, soll die Kommunikation in der Schleife weiterlaufen
	 */
	checkServerReply(sock, buffer, shm);

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
			/* Fall das Socket ready to read ist */
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

			/* Fall das Pipe ready to read ist */
			} else if (FD_ISSET(pipe, &fds)) {
				doMove(sock, buffer, fd);
			}

		/* Falls select 0 returned heisst die Wartezeit ist ohne Ereigniss abgelaufen */
		} else if (rc == 0) {
			perror("Select timed out!. \n");
		}
	}
	while (rc != 0);

	return EXIT_SUCCESS;
}
