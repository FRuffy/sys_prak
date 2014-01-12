#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "SharedVariables.h"
#include "Errmmry.h"

char* recvServerInfo(char* buffer,sharedmem * shm)
{
    char* buffer2;
    char* bfrptr; //Wichtig, falls strtok zur Modifikation des Strings verwendet wird.Der Pointer ändert sich im Verlauf, die geschriebenen Daten aber nicht.
    buffer2 = malloc(sizeof(char)*256);
    bfrptr = buffer2;   addchar(bfrptr);
    int i = 1;
    strcpy(buffer2,buffer);
     buffer2 = strtok( buffer2, "\n" );

    //Verarbeite: "+ TOTAL 2" (Anzahl der Spieler)
    sscanf(buffer2, "%*s %*s %i", &(shm->playerCount));
    if (shm->playerCount > 8)
    {
        printf("\nDiese Version des Clients unterstuetzt maximal 8 Spieler, die Anzahl ist jedoch %d Spieler!\n",shm->playerCount);

        return NULL;
    }

    //Verarbeite: "+ 1 GEGENSPIELERNAME 0" (Uebermittlung der Gegenspieler
    //(Nummer, Name, Flag ob bereit) solange bis Server "+ ENDPLAYERS" sendet)
    buffer2 = strtok( NULL, "\n" );
    while (strcmp(buffer2,"+ ENDPLAYERS") != 0)
    {
        sscanf(buffer2, "%*s %d %s %i", &shm->player[i].playerNumber, shm->player[i].playerName, &shm->player[i].playerReady);
        buffer2 = strtok( NULL, "\n" );
        i++;
    }
    printf("\nEs spielen diese %i Spieler:\n", shm->playerCount);
    printf("\nSpielernummer\tSpielername\tBereit\n");

    for (i=0; i<shm->playerCount; i++)
    {
        printf("%d\t\t%s\t\t%d\n", shm->player[i].playerNumber, shm->player[i].playerName, shm->player[i].playerReady);
    }
    buffer2 = strtok( NULL, "" );
    strcpy(buffer, buffer2);
    return buffer;
}
