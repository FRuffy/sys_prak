#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "SharedVariables.h"
#include "Errmmry.h"
#include "Select.h"
#include "InitConnection.h"
#define BUFFR 512

/**
 * Haengt \n an den String fuer den Server an
 *
 * @param Socket, Pointer auf String wo das \n dran soll
 * @return Pointer auf String mit angehaengtem \n
 */
void sendReplyFormatted(int sock, char* reply) {
	char * container;
	container = malloc(sizeof(char) * (strlen(reply) + 2));
	strcpy(container, reply);
	strcat(container, "\n");
	send(sock, container, strlen(container), 0);
	free(container);
}

/**
 * Umgedrehte strcat Funktion um custom strings zu uebergeben, die fuer die korrekte uebertragung noetig sind
 *
 * @param Pointer
 * @return 0
 */
int antistrcat(char* dest, char* src, char* temp) {
	strcpy(temp, src);
	strcat(temp, dest);
	return EXIT_SUCCESS;
}

/**
 * Formatierte Ausgabe an den Server (\n wird benoetigt um das Ende der Uebertragung zu signalisieren)
 * Ueberpruefung und Uebertragung der Client Version, Game-ID, Spielernummer
 *
 * @param Socket, Struktur, SHM, pipe
 * @return 0
 */
int performConnection(int sock, sharedmem * shm, config_struct* conf, int fd[]) {
	int err;
	char* reader = malloc(sizeof(char) * 20);
	addchar(reader);
	char* temp = malloc(sizeof(char) * BUFFR);
	addchar(temp);
	char* buffer = malloc(sizeof(char) * BUFFR);
	addchar(buffer);

	/* Teil 1: Lese die Client-Version des Servers und antworte mit eigener (formatierten) Version; 
	 * Behandle die Antwort des Servers */
	err = recv(sock, buffer, BUFFR - 1, 0);
	writelog(logdatei, AT);
	sscanf(buffer, "%*s%*s%*s%s", reader);
	if (err > 0) {
		buffer[err] = '\0';
	}

	printf("\nDie Version des Servers ist: %s\n", reader);
	antistrcat(conf->version, "VERSION ", temp);
	sendReplyFormatted(sock, temp);
	err = recv(sock, buffer, BUFFR - 1, 0);
	writelog(logdatei, AT);
	if (err > 0) {
		buffer[err] = '\0';
	}

	if (buffer[0] == '-') {
		printf("\nDer Server akzeptiert die Version %s dieses Clients nicht!\n", conf->version);
		return EXIT_FAILURE;
	} else {
		printf("\nDie Client-Version wurde akzeptiert, uebertrage Spiel-ID...\n");
	}

	/* Teil 2: Der Server erwartet Game-ID, schicke diese und behandle Antwort.
	 * Falls die ID fehlerhart oder das Spiel des Servers nicht Quarto ist wird die Verbindung beendet */
	antistrcat(shm->gameID, "ID ", temp);
	sendReplyFormatted(sock, temp);
	err = recv(sock, buffer, BUFFR - 1, 0);
	writelog(logdatei, AT);
	if (err > 0) {
		buffer[err] = '\0';
	}
	sscanf(buffer, "%*s%*s%s", reader);

	if (buffer[0] == '-') {
		printf("\nDie uebergebene Game-ID fehlt oder ist fehlerhaft! Beende Verbindung\n");
		return EXIT_FAILURE;
	} else if (strcmp(reader, "Quarto") != 0) {
		printf("\nDas Spiel, das der Server spielt, ist nicht Quarto! Beende Verbindung.\n");
		return EXIT_FAILURE;
	} else {
		printf("\nDer Server moechte %s spielen. Und wir auch!\n", reader);
	}

	/* Teil 3.1: Hatten wir mit unserer ID Erfolg erfahren wir zunaechst den Namen des Spiels und 
	 * senden die ggf. uebergebene Spielernummer. Anschliessend wird die Antwort des Servers auf 
	 * die Nummer behandelt und Name und Nummer des Spielerplatzes ausgelesen */
	err = recv(sock, buffer, BUFFR - 1, 0);
	writelog(logdatei, AT);
	if (err > 0) {
		buffer[err] = '\0';
	}

	sscanf(buffer, "%*s%s", shm->gameName);
	printf("\nSpiel: %s\n", shm->gameName); //Zeige Spielnamen an, schneide das "+" ab
	antistrcat(conf->playernumber, "PLAYER ", temp);
	sendReplyFormatted(sock, temp);
	err = recv(sock, buffer, BUFFR - 1, 0);
	writelog(logdatei, AT);
	if (err > 0) {
		buffer[err] = '\0';
	}
	if (buffer[0] == '-') {
		if (buffer[2] == '-') {
			printf("\nEs wurde eine ungueltige Spielernummer eingegeben! Beende Verbindung\n");
		} else {
			printf("\nEs wurde kein freier Platz gefunden, versuchen sie es spaeter noch einmal!\n");
			/* ACHTUNG. Diese Meldung erscheint auch, wenn man in der client.conf eine Spielernummer an gibt, 
			 * die beim Erstellen des Spiels einem Computerspieler zugeordnet wurde! */
		}
		return EXIT_FAILURE;
	} else {
		sscanf(buffer, "%*s %*s %d %[^\n]\n", &(shm->player[0].playerNumber),
				(shm->player[0].playerName));
	}

	/* Teil 3.2: Hier wird der Uebergang in die Spielverlaufsphase eingeleitet.
	 * Der Server sendet uns die Namen unseres Gegenspielers und der Spielernummer, wobei ueberprueft wird, 
	 * ob ein Spieler bereits verbunden ist. */
	err = recv(sock, buffer, BUFFR - 1, 0);
	writelog(logdatei, AT);
	if (err > 0) {
		buffer[err] = '\0';
	}
	if (buffer[0] == '-') {
		printf("\nFehler bei Uebermittlung der Spielparameter!\n");
		return EXIT_FAILURE;
	}

	/* Empfange die Serverdaten, falls ein Fehler hier auftritt Programm beenden */
	if (recvServerInfo(buffer, shm) == NULL ) {
		return EXIT_FAILURE;
	}

	/* Hier faengt im Endeffekt der Connector an, der die laufende Verbindung mit dem Server haendelt und 
	 * mit dem Thinker zusammenarbeitet */
	if (waitforfds(sock, buffer, shm, fd) != 0) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * Verarbeite erstes Input vom Server bis ENDPLAYERS
 *
 * @param buffer und Shared Memory shm
 * @return 0 falls Verbindung hergestellt und performConnection ausfuehrbar
 */
char* recvPlayerInfo(char* buffer, sharedmem * shm) {
	char* buffer2;
	buffer2 = malloc(sizeof(char) * 256);
	addchar(buffer2);
	int i = 1;
	strcpy(buffer2, buffer);
	buffer2 = strtok(buffer2, "\n");

	/* Verarbeite: "+ TOTAL 2" (Anzahl der Spieler) */
	sscanf(buffer2, "%*s %*s %i", &(shm->playerCount));
	if (shm->playerCount > 8) {
		printf("\nDiese Version des Clients unterstuetzt maximal 8 Spieler, die Anzahl ist jedoch %d Spieler!\n", shm->playerCount);
		return NULL ;
	}

	/* Verarbeite: "+ 1 GEGENSPIELERNAME 0" (Uebermittlung der Gegenspieler
	 * (Nummer, Name, Flag ob bereit) solange bis Server "+ ENDPLAYERS" sendet) */
	int rdy = -1;
	char* temp = malloc(sizeof(char) * 50);
	addchar(temp);
	buffer2 = strtok(NULL, "\n");

	while (strcmp(buffer2, "+ ENDPLAYERS") != 0) {
		sscanf(buffer2, "%*s %d %s %d %d", &shm->player[i].playerNumber, shm->player[i].playerName, &shm->player[i].playerReady, &rdy);

		if (rdy != -1) {
			snprintf(temp, 50, " %d", shm->player[i].playerReady);
			strcpy(shm->player[i].playerName, strcat(shm->player[i].playerName, temp));
			shm->player[i].playerReady = rdy;
		}

		buffer2 = strtok(NULL, "\n");
		i++;
	}

	printf("\nWir sind Spieler: %s\n", shm->player[0].playerName);
	printf("\nInsgesamt spielen diese %i Spieler:\n", shm->playerCount);
	printf("\n%-15s %-25s %-2s\n", "Spielernummer", "Spielername", "Bereit");
	/* Wir selbst sind natuerlich immer bereit. */
	printf("%-15d %-25s %-2d\n", shm->player[0].playerNumber, shm->player[0].playerName, 1);

	/* Funktioniert eigentlich aber Server scheint hier unberechenbare Spielerinfo zu senden */
	for (i = 1; i < shm->playerCount; i++) {
		printf("%-15d %-25s %-2d\n", shm->player[i].playerNumber, shm->player[i].playerName, shm->player[i].playerReady);
		printf("=================================================\n");
	}

	printf("\n");
	buffer2 = strtok(NULL, "");
	strcpy(buffer, buffer2);

	return buffer;
}
