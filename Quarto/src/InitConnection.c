#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include "SharedVariables.h"
#include "Errmmry.h"
#include "PerformConnection.h"

/**
 * Herstellung der Verbindung zum Server
 *
 * @param Pointer auf SHM, Struktur und pipe
 * @return 0 falls Verbindung hergestellt und performConnection ausfuehrbar
 */
int initConnection(sharedmem * shm, config_struct* conf, int fd[]) {
	shm->sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in host;
	struct hostent* ip;

	/* Uebersetzt Hostname in IP Adresse */
	ip = (gethostbyname(conf->hostname));
	if (ip == NULL ) {
		perror("\nFehler beim Anfordern der IP");
		writelog(logdatei, AT);
	}

	memcpy(&(host.sin_addr), ip->h_addr,ip ->h_length);
	host.sin_family = AF_INET;
	host.sin_port = htons(conf->portnumber);

	if (connect(shm->sock, (struct sockaddr*) &host, sizeof(host)) == 0) {
		printf("\nVerbindung mit %s hergestellt!\n", conf->hostname);
		writelog(logdatei, AT);
	} else {
		perror("\n Fehler beim Verbindungsaufbau");
		return EXIT_FAILURE;
	}

	/* PID in SHM schreiben */
	shm->pidDad = getppid();
	shm->pidKid = getpid();

	/*Fuehre Prolog Protokoll aus */
	if (performConnection(shm->sock, shm, conf, fd) != 0) {
		close(shm->sock);
		return EXIT_FAILURE;
	}

	close(shm->sock);
	return EXIT_SUCCESS;
}
