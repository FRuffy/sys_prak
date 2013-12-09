#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include "sharedVariables.h"
#include "errmmry.h"

#define BUFFR 256
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

/**
* readGameField bekommt folgendes uebergeben und liesst es in den pf-SHM ein
* + 4 * * 15 *
* + 3 * * * *
* + 2 * * * *
* + 1 * * * *
* + ENDFIELD
*/
int readGameField(char *buffer2)
{
    int i=0, znr=0;

    while (strcmp(buffer2,"+ ENDFIELD") != 0)
    {
//Kommentarzeichen entfernen um zu kapieren wie die Funktion funktioniert!
//         printf("\n%s\n", buffer2);
        sscanf (buffer2,"%*s %d %[0-9* ]", &znr, buffer2);

        for (i=0; i<=shm->fieldX; i++)
        {
            *(pf+i+(znr-1)*(shm->fieldX)) = -1;
            sscanf (buffer2,"%d %[0-9* ]",(pf+i+(znr-1)*(shm->fieldY)), buffer2);
//                         printf("=>%d \n", *(pf+i+(znr-1)*shm->fieldY));
            sscanf (buffer2,"%*s %[0-9* ]",buffer2);
//                         printf(": %s \n", buffer2);
        }
        buffer2 = strtok( NULL, "\n" );
    }

    return EXIT_SUCCESS;
}

/**
* printGameField gibt aktuellen Zustand des Spielfeldes aus
*/
int printGameField()
{
    int i,j;
    for (i=shm->fieldY-1; i>=0; i--)
    {
        printf("\n %d: ", i+1);
        for (j=0; j<shm->fieldX; j++)
        {
            if (*(pf+j+i*shm->fieldY)==-1)
            {
                printf(" *");
            }
            else
            {
                printf(" %d", *(pf+j+i*shm->fieldY));
            }
        }
    }
    printf("\n     A B C D\n");
    return EXIT_SUCCESS;
}


int recvServerInfo(char* buffer)
{

    char* buffer2;
    char* bufrptr;
    buffer2 = malloc(sizeof(char)*512);
    bufrptr = buffer2;
    int i = 0;

    /* sscanf(buffer,"%*s %*s %d %*s %d %s %i %*[^\n] %*s %*s %d %*s %*s %d %*s %*s %d%*[,]%d"
            ,&(shm->playerCount),&shm->player[i].playerNumber, &shm->player[i].playerName, &shm->player[i].playerReady, &(shm->thinkTime), &(shm->nextStone)
            ,&(shm->fieldX), &(shm->fieldY)); */

    strcpy(buffer2,buffer);
    bufrptr = buffer2;

    printf("\n%s\n",buffer2);
    buffer2 = strtok( buffer2, "\n" );
    printf("\n%s\n",buffer2);

    //Verarbeite: "+ TOTAL 2" (Anzahl der Spieler)
    sscanf(buffer2, "%*s %*s %i", &(shm->playerCount));
    if (shm->playerCount > 8)
    {
        printf("\nDiese Version des Clients unterstuetzt maximal 8 Spieler!\n");
        free(bufrptr);
        return EXIT_FAILURE;
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
    buffer2 = strtok( NULL, "\n" );

    //Verarbeite: "+ MOVE 3000" (Zeit zur uebermittlung des naechsten Zuges)
    sscanf(buffer2, "%*s %*s %d", &(shm->thinkTime));
    printf("\nFuer deinen Zug hast du %d ms, ",shm->thinkTime);
    buffer2 = strtok( NULL, "\n" );

    //Verarbeite: "+ NEXT 4" (Naechster zu setzender Spielstein)
    sscanf(buffer2, "%*s %*s %d", &(shm->nextStone));
    printf("du musst Stein %d setzen!\n\n",shm->nextStone);
    buffer2 = strtok( NULL, "\n" );

    //Verarbeite: "+ FIELD 4,4" (Spielfeldgroesse)
    sscanf(buffer2, "%*s %*s %d%*[,]%d", &(shm->fieldX), &(shm->fieldY));
    printf("Unser momentanes Spielfeld. Größe: %d x %d\n",shm->fieldX, shm->fieldY);

    //Wir kennen jetzt die Spielfeldgroesse => SHM-pf (Playing Field) dafuer reservieren und einhaengen (2x Groesse von fieldX wegen 4 Merkmalen pro Stein!)
    pfID = shmget(IPC_PRIVATE, (sizeof(short)*(shm->fieldX)*(shm->fieldX)*(shm->fieldY)), IPC_CREAT | IPC_EXCL | 0775);
    if (pfID < 1)
    {
        printf("Error: No pf-SHM");
        free(bufrptr);

        return EXIT_FAILURE;
    }
    pf = shmat(pfID, 0, 0); //pf einhaengen
    if (pf == (void *) -1) //Im Fehlerfall pointed pf auf -1
    {
        fprintf(stderr, "Fehler, pf-shm: %s\n", strerror(errno));
        writelog(logdatei,AT);
    }
    buffer2 = strtok( NULL, "\n" );

    readGameField(buffer2);
    printGameField();
    free(bufrptr);
    return EXIT_SUCCESS;
}
