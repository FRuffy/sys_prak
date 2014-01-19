#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>
#include "SharedVariables.h"
#include "Errmmry.h"

#define BUFFR 512

/**
 * readGameField bekommt folgendes uebergeben und liesst es in den pf-SHM ein
 * + 4 * * * *
 * + 3 * * * *
 * + 2 * * * *
 * + 1 * * * *
 * + ENDFIELD
 * Zeile 51: buffer2 wird mit strtok veraendert, fuer free wird eine Kopie benoetigt
 *
 * @param Puffer, SHM
 * @return 0 (Eingelesenes Spielfeld in *pf)
 */
int readGameField(char *buffer, sharedmem * shm) {
	char* buffer2;
	buffer2 = malloc(sizeof(char) * 128);
	char *buffer2temp = buffer2;
	int i = 0, znr = 0;
	char tmp = '*';
	buffer2 = strstr(buffer, "+ FIELD");
	buffer2 = strtok(buffer2, "\n");
	buffer2 = strtok(NULL, "\n");

	while (strcmp(buffer2, "+ ENDFIELD") != 0) {
		sscanf(buffer2, "%*s %d %[0-9* ]", &znr, buffer2);

		for (i = 0; i < shm->fieldX; i++) {
			*(shm->pf + i + ((znr - 4) * (-1)) * shm->fieldX) = -1;
			if (buffer2 == strchr(buffer2, tmp)) {
				sscanf(buffer2, "%*s %[0-9* ]", buffer2);
			} else {
				sscanf(buffer2, "%d %[0-9* ]",
						(shm->pf + i + ((znr - 4) * (-1)) * (shm->fieldY)),
						buffer2);
			}
		}
		buffer2 = strtok(NULL, "\n");
	}

	free(buffer2temp);
	return EXIT_SUCCESS;
}

/**
 * Gibt Spielfeld aus
 *
 * @param Pointer auf SHM
 * @return 0 (Spielfeld auf Konsole)
 */
int printGameField(sharedmem * shm) {
	int i, j;

	for (i = 0; i < shm->fieldY; i++) {
		printf("\n %d: ", i + 1);
		for (j = 0; j < shm->fieldX; j++) {
			if (*(shm->pf + j + i * shm->fieldY) == -1) {
				printf(" *");
			} else {
				printf(" %d ", *(shm->pf + j + i * shm->fieldY));
			}
		}
	}

	return EXIT_SUCCESS;
}

/**
 * Ueberprueft den erhaltenen Buffer auf eventuell auftretende negative Servermeldungen
 *
 * @param Socket, Buffer
 * @return 0 falls keine negative Servermeldung
 */
int handleRecv(int sock, char* buffer) {
	int err;
	err = recv(sock, buffer, BUFFR - 1, 0);

	if (err > 0) {
		buffer[err] = '\0';
	}

	if (buffer[0] == '-') {
		printf("\nFehler in der Kommunikation, der Server meldet: %s \n",
				buffer);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * Ueberprueft die Antworten vom Server
 *
 * @param Socket, Buffer, SHM
 * @return 0 falls alle Antworten ok, bzw. 2 falls Spiel beendet wurde
 */
int checkServerReply(int sock, char* buffer, sharedmem * shm) {
	if (buffer[0] == '-') {
		printf("\nFehler in der Kommunikation, der Server meldet: %s \n",
				buffer);
		return EXIT_FAILURE;
	}

	/* Falls WAIT zurueckgegeben wird. */
	if (strncmp(buffer, "+ MOVE", 6) == 0 && (strlen(buffer) < 15)) {
		sscanf(buffer, "%*s %*s %d", &(shm->thinkTime));
		if (handleRecv(sock, buffer) != 0) {
			return EXIT_FAILURE;
		}
		sscanf(buffer, "%*s %*s %d %*s %*s %d%*[,]%d", &(shm->StoneToPlace),
				&(shm->fieldX), &(shm->fieldY));
		printf("\nFuer deinen Zug hast du %d ms und ", shm->thinkTime);

	} else if (strcmp(buffer, "+ WAIT\n") == 0) {
		do {
			printf("\nWarte auf andere Spieler.\n");
			send(sock, "OKWAIT\n", strlen("OKWAIT\n"), 0);
			if (handleRecv(sock, buffer) != 0) {
				return EXIT_FAILURE;
			}
		} while (strcmp(buffer, "+ WAIT\n") == 0);

		/* Ueberprueft wer gewonnen hat, falls das Spiel zu ende ist! */
		if (strncmp(buffer, "+ GAMEOVER", 10) != 0) {
			if (handleRecv(sock, buffer) != 0) {
				return EXIT_FAILURE;
			}
			printf("\nBuffer2: %s\n", buffer);
		}
		sscanf(buffer, "%*s %*s %d %*s %*s %d%*[,]%d", &(shm->StoneToPlace),
				&(shm->fieldX), &(shm->fieldY));

	} else {
		sscanf(buffer, "%*s %*s %d %*s %*s %d %*s %*s %d%*[,]%d",
				&(shm->thinkTime), &(shm->StoneToPlace), &(shm->fieldX),
				&(shm->fieldY));
	}

	if (strstr(buffer, "+ GAMEOVER") != NULL ) {
		int playerNumber;
		char playerName[BUFFR];
		printf("\nBuffer3: %s\n", buffer);

		if (strncmp(buffer, "+ MOVEOK", 8) == 0) {

			buffer = strtok(buffer, "\n");
			buffer = strtok(NULL, "\n");
		}

		if (strcmp(buffer, "+ GAMEOVER\n") == 0
				|| strncmp(buffer, "+ GAMEOVER\n+", 12) == 0) {
			printf("\nDas Spiel ist zu Ende. Es gibt keinen Gewinner.\n");
		} else {
			sscanf(buffer, "%*s %*s %d %s", &playerNumber, playerName);
			printf("\nDas Spiel ist zu Ende. Der Gewinner ist: %d - %s\n",
					playerNumber, playerName);
		}
		return 2;
	}

	/* Ueberprueft ob der Server einfach nur die Verbindung beenden will */
	if (strstr(buffer, "+ QUIT") != NULL ) {
		printf("\nDas Spiel ist zu Ende.\n");
		return 2;
	}

	printf("Stein %d ist zu setzen!\n", shm->StoneToPlace);
	printf("\nUnser momentanes Spielfeld mit Groesse %d x %d:\n", shm->fieldX,
			shm->fieldY);

	/* Wir kennen jetzt die Spielfeldgroesse => SHM-pf (Playing Field) dafuer reservieren und einhaengen (2x Groesse von fieldX wegen 4 Merkmalen pro Stein!) */
	if (shm->pfID == 0) {
		shm->pfID = shmget(KEY,
				(sizeof(short) * (shm->fieldX) * (shm->fieldX) * (shm->fieldY)),
				IPC_CREAT | 0775);
		if (shm->pfID < 1) {
			writelog(logdatei, AT);
			perror("KIND");
			return EXIT_FAILURE;
		}
	}

	/* Playing Field einhaengen */
	shm->pf = shmat(shm->pfID, 0, 0);

	/* Im Fehlerfall pointed pf auf -1 */
	if (*(shm->pf) == -1) {
		fprintf(stderr, "Fehler, pf-shm: %s\n", strerror(errno));
		writelog(logdatei, AT);
	}

	readGameField(buffer, shm);

	/* Bereitet den naechsten Zug vor (sendet Signal an Vater-Thinker) */
	sendReplyFormatted(sock, "THINKING");
	if (handleRecv(sock, buffer) != 0) {
		return EXIT_FAILURE;
	}

	if (strcmp(buffer, "+ OKTHINK\n") != 0) {
		if (handleRecv(sock, buffer) != 0) {
			return EXIT_FAILURE;
		}
	}

	/* Naechsten Spielzug ausdenken (ueber thinker) */
	shm->thinking = 1;
	shm->pleaseThink = 1;

	/* Sagt dem Vater, dass er jetzt denken soll */
	kill(shm->pidDad, SIGUSR1);

	return EXIT_SUCCESS;
}
