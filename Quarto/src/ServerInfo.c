#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "SharedVariables.h"
#include "Errmmry.h"

/**
 * Verarbeite erstes Input vom Server bis ENDPLAYERS
 *
 * @param buffer und Shared Memory shm
 * @return 0 falls Verbindung hergestellt und performConnection ausfuehrbar
 */
char* recvServerInfo(char* buffer, sharedmem * shm) {
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
