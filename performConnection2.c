#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "sharedVariables.h"
#include "errmmry.h"


char* recvServerInfo(char* buffer)
{

    char* buffer2;
    char* bufrptr;
    buffer2 = malloc(sizeof(char)*256);
    bufrptr = buffer2;
    int i = 1;

    /* sscanf(buffer,"%*s %*s %d %*s %d %s %i %*[^\n] %*s %*s %d %*s %*s %d %*s %*s %d%*[,]%d"
            ,&(shm->playerCount),&shm->player[i].playerNumber, &shm->player[i].playerName, &shm->player[i].playerReady, &(shm->thinkTime), &(shm->nextStone)
            ,&(shm->fieldX), &(shm->fieldY)); */

    strcpy(buffer2,buffer);
    bufrptr = buffer2;

    printf("\n%s\n",buffer2);
    buffer2 = strtok( buffer2, "\n" );


    //Verarbeite: "+ TOTAL 2" (Anzahl der Spieler)
    sscanf(buffer2, "%*s %*s %i", &(shm->playerCount));
    if (shm->playerCount > 8)
    {
        printf("\nDiese Version des Clients unterstuetzt maximal 8 Spieler!\n");
        free(bufrptr);
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
    //printf("\n%s\n",buffer2);

    free(bufrptr);

    return buffer2;

}
