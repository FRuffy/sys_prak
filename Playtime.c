#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <errno.h>
#include "sharedVariables.h"
#include "errmmry.h"
#include "auxiliaryFunctions.h"

#define BUFFR 512

int checkServerReply(int sock, char* buffer, sharedmem * shm) {
	int size;

// if then else, falls WAIT zurueckgegeben wird.

if (strncmp(buffer,"+ MOVE", 6) == 0 && (strlen(buffer)< 15)) {
size = recv(sock, buffer, BUFFR - 1, 0);
		if (size > 0)
			buffer[size] = '\0';
	sscanf(buffer,"%*s %*s %d %*s %*s %d%*[,]%d", &(shm->StoneToPlace),&(shm->fieldX), &(shm->fieldY));
	printf("\nFuer deinen Zug hast du %d ms und ",shm->thinkTime);

}
else
if(strcmp(buffer,"+ WAIT\n") == 0)
{
	do
	{
		printf("\nWarte auf andere Spieler.\n");
		send(sock,"OKWAIT\n",strlen("OKWAIT\n"),0);
		size = recv(sock, buffer, BUFFR-1, 0);
		if (size > 0) buffer[size]='\0';

	}
	while (strcmp(buffer,"+ WAIT\n") == 0);
	size = recv(sock, buffer, BUFFR-1, 0);
	if (size > 0) buffer[size]='\0';
	sscanf(buffer,"%*s %*s %d %*s %*s %d%*[,]%d", &(shm->StoneToPlace),&(shm->fieldX), &(shm->fieldY));

} else if (strncmp(buffer,"+ GAMEOVER", 10) == 0) {
	if (strcmp(buffer,"+ GAMEOVER\n") == 0) {
		printf("\nDas Spiel ist zu ende. Es gibt keinen Gewinner.\n");
	} else {
		int playerNumber;
		char playerName[BUFFR];
		sscanf(buffer, "%*s %*s %d %s", &playerNumber, playerName);
		printf("\nDas Spiel ist zu ende. Der Gewinner ist: %d - %s\n", playerNumber, playerName);
		//Return 2, fuer Spiel beendet. Looper gibt dann EXIT_SUCCESS(1) zurueck.
		return 2;
	}
}
else
{
	sscanf(buffer,"%*s %*s %d %*s %*s %d %*s %*s %d%*[,]%d",&(shm->thinkTime), &(shm->StoneToPlace),&(shm->fieldX), &(shm->fieldY));
	printf("\nFuer deinen Zug hast du %d ms und ",shm->thinkTime);
}

	/* BEISPIELOUTPUT SERVER START
	 + MOVE 3000
	 + NEXT 8
	 + FIELD 4,4
	 + 4 12 4 15 *
	 + 3 3 10 * *
	 + 2 9 * * *
	 + 1 13 * * *
	 + ENDFIELD

	 vs.

	 BEISPIELOUTPUT VOM SERVER WAEHREND SPIEL
	 + NEXT 5
	 + FIELD 4,4
	 + 4 12 4 15 11
	 + 3 3 10 8 *
	 + 2 9 2 * 1
	 + 1 13 * 6 14
	 + ENDFIELD

	 */
/*	  BEISPIELOUTPUT VOM SERVER SPIEL ENDE
	 + NEXT 5
	 + FIELD 4,4
	 + 4 12 4 15 11
	 + 3 3 10 8 *
	 + 2 9 2 * 1
	 + 1 13 * 6 14
	 + ENDFIELD

	 */

	printf("Stein %d ist zu setzen!\n\n",shm->StoneToPlace);
	printf("Unser momentanes Spielfeld. Groesse: %d x %d\n",shm->fieldX, shm->fieldY);

	/* @FLO bitte eine if clause Einfuegen die checkt ob pfID bereits existiert, falls ja die Erstellung ignorieren.
	 Die Funktion wird warscheinlich oft benutzt werden */
printf("\n%d\n",shm->pfID);

	//Wir kennen jetzt die Spielfeldgroesse => SHM-pf (Playing Field) dafuer reservieren und einhaengen (2x Groesse von fieldX wegen 4 Merkmalen pro Stein!)
	if (shm->pfID == 0) {
		shm->pfID = shmget(KEY, (sizeof(short)*(shm->fieldX)*(shm->fieldX)*(shm->fieldY)),IPC_CREAT  | 0775 ); 	writelog(logdatei,AT);
		if (shm->pfID < 1)
				{
			printf("KIND: Error: No pf-SHM");
			return EXIT_FAILURE;
		}
		shm->pf = shmat(shm->pfID, 0, 0); writelog(logdatei,AT); //pf einhaengen
	}

	if (shm->pf == (void *) -1)//Im Fehlerfall pointed pf auf -1
	{
		fprintf(stderr, "Fehler, pf-shm: %s\n", strerror(errno));
		writelog(logdatei,AT);
	}

	readGameField(buffer, shm);

	return EXIT_SUCCESS;
}
