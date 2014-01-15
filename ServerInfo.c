#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "SharedVariables.h"
#include "Errmmry.h"

/**
 * Informationen ueber Spieler
 *
 * @param Pointer auf Buffer, SHM
 * @return Info ueber Spielernummer, Spielername, Bereit
 */
char* recvServerInfo(char* buffer, sharedmem * shm) {
	char* buffer2;
	char* bfrptr; // Wichtig, falls strtok zur Modifikation des Strings verwendet wird. Der Pointer aendert sich im Verlauf, die geschriebenen Daten aber nicht
	buffer2 = malloc(sizeof(char) * 256);
	bfrptr = buffer2;
	addchar(bfrptr);
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
	buffer2 = strtok(NULL, "\n");
	while (strcmp(buffer2, "+ ENDPLAYERS") != 0) {
		sscanf(buffer2, "%*s %d %s %i", &shm->player[i].playerNumber, shm->player[i].playerName, &shm->player[i].playerReady); buffer2 = strtok(NULL, "\n");
		i++;
	}
	printf("\nWir sind Spieler: %s\n", shm->player[0].playerName);
	printf("\nInsgesamt spielen diese %i Spieler:\n", shm->playerCount);
	printf("\n%-15s %-25s %-2s\n", "Spielernummer", "Spielername", "Bereit");
	/* Wir selbst sind natürlich immer bereit. */
	printf("%-15d %-25s %-2d\n", shm->player[0].playerNumber, shm->player[0].playerName, 1);

	// Funktioniert eigentlich aber Server hat hier anscheinend einen Bug und sendet falsche Spielernummer
	for (i = 1; i < shm->playerCount; i++) {
		printf("%-15d %-25s %-2d\n", shm->player[i].playerNumber, shm->player[i].playerName, shm->player[i].playerReady);
	}

	printf("\n");
	buffer2 = strtok(NULL, "");
	strcpy(buffer, buffer2);
	return buffer;
}
