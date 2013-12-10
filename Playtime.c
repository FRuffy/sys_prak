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
/* Diese Funktion funktioniert NICHT während eines Spieles, da der Server hier andere Felder zurückschickt"
 Es scheint, dass der Connector diese Schliefen übernehmen muss.
 Evtl diese Funktion einfach in Serverinfo übernehmen und dann mit einer anderen weiterarbeiten.
*/
/*Edit:
Habs hinbekommen, diese Funktion funktioniert jetzt auch während des Spieles und kann vom Connector zur Kommunikation 
verwendet werden. Der untere auskommentierte Teil ist die alte Version, falls jemand nachvollziehen möchte was hier 
hier eigentlich gemacht wird */

int checkServerReply(int sock, char* buffer)
{
    int size;
// if then else, falls WAIT zurücgegeben wird.
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
        sscanf(buffer,"%*s %*s %d %*s %*s %d%*[,]%d", &(shm->nextStone),&(shm->fieldX), &(shm->fieldY));

    }
    else
    {
        sscanf(buffer,"%*s %*s %d %*s %*s %d %*s %*s %d%*[,]%d",&(shm->thinkTime), &(shm->nextStone),&(shm->fieldX), &(shm->fieldY));
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

    printf("Stein %d ist zu setzen!\n\n",shm->nextStone);
    printf("Unser momentanes Spielfeld. Groesse: %d x %d\n",shm->fieldX, shm->fieldY);

    /* @FLO bitte eine if clause Einfügen die checkt ob pfID bereits existiert, falls ja die Erstellung ignorieren.
    Die Funktion wird warscheinlich oft benutzt werden */

    //Wir kennen jetzt die Spielfeldgroesse => SHM-pf (Playing Field) dafuer reservieren und einhaengen (2x Groesse von fieldX wegen 4 Merkmalen pro Stein!)
    pfID = shmget(IPC_PRIVATE, (sizeof(short)*(shm->fieldX)*(shm->fieldX)*(shm->fieldY)), IPC_CREAT | IPC_EXCL | 0775);
    if (pfID < 1)
    {
        printf("Error: No pf-SHM");


        return EXIT_FAILURE;
    }
    pf = shmat(pfID, 0, 0); //pf einhaengen
    if (pf == (void *) -1) //Im Fehlerfall pointed pf auf -1
    {
        fprintf(stderr, "Fehler, pf-shm: %s\n", strerror(errno));
        writelog(logdatei,AT);
    }


    readGameField(buffer);
    printGameField();

    return EXIT_SUCCESS;
}
/*
 char* buffer2;
 char* bufrptr;
 buffer2 = malloc(sizeof(char)*512);
 bufrptr = buffer2;

sscanf(buffer,"%*s %*s %d %*s %d %s %i %*[^\n] %*s %*s %d %*s %*s %d %*s %*s %d%*[,]%d"
         ,&(shm->playerCount),&shm->player[i].playerNumber, &shm->player[i].playerName, &shm->player[i].playerReady, &(shm->thinkTime), &(shm->nextStone)
         ,&(shm->fieldX), &(shm->fieldY));

 strcpy(buffer2,buffer);
 bufrptr = buffer2;

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
 printf("Unser momentanes Spielfeld. Groesse: %d x %d\n",shm->fieldX, shm->fieldY);

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
*/
