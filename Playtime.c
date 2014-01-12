#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>
#include "SharedVariables.h"
#include "Errmmry.h"
#include "AuxiliaryFunctions.h"

#define BUFFR 512

/**
* Ueberprueft den erhaltenen Buffer auf eventuell
* auftretende negative Servermeldungen
**/
int handleRecv(int sock, char* buffer) {
     int err;
     err = recv(sock, buffer, BUFFR - 1, 0);
        if (err > 0) {
            buffer[err] = '\0';
        }
  if (buffer[0] == '-') {
        printf("\nFehler in der Kommunikation, der Server meldet: %s \n", buffer );
        return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}



int checkServerReply(int sock, char* buffer, sharedmem * shm) {

     if (buffer[0] == '-') {
        printf("\nFehler in der Kommunikation, der Server meldet: %s \n", buffer );
        return EXIT_FAILURE;
    }

   /* if then else, falls WAIT zurueckgegeben wird. */
    if (strncmp(buffer,"+ MOVE", 6) == 0 && (strlen(buffer)< 15)) {
        sscanf(buffer,"%*s %*s %d",&(shm->thinkTime));
        if (handleRecv(sock,buffer) != 0)
        {
            return EXIT_FAILURE;
        }
        sscanf(buffer,"%*s %*s %d %*s %*s %d%*[,]%d", &(shm->StoneToPlace),&(shm->fieldX), &(shm->fieldY));
        printf("\nFuer deinen Zug hast du %d ms und ",shm->thinkTime);
    }  else if(strcmp(buffer,"+ WAIT\n") == 0)
    {
        do
        {
            printf("\nWarte auf andere Spieler.\n");
            send(sock,"OKWAIT\n",strlen("OKWAIT\n"),0);
            if (handleRecv(sock,buffer) != 0)
        {
            return EXIT_FAILURE;
        }

        }
        while (strcmp(buffer,"+ WAIT\n") == 0);

// Ueberprueft wer gewonnen hat, falls das Spiel zu ende ist!

        if (strncmp(buffer,"+ GAMEOVER", 10) != 0)
        {
            if (handleRecv(sock,buffer) != 0)
        {
            return EXIT_FAILURE;
        }
            printf("\nBuffer2: %s\n", buffer);

        }
        sscanf(buffer,"%*s %*s %d %*s %*s %d%*[,]%d", &(shm->StoneToPlace),&(shm->fieldX), &(shm->fieldY));

    } else {
        sscanf(buffer,"%*s %*s %d %*s %*s %d %*s %*s %d%*[,]%d",&(shm->thinkTime), &(shm->StoneToPlace),&(shm->fieldX), &(shm->fieldY));
    }


    if (strstr(buffer, "+ GAMEOVER")!= NULL) {
        if (strcmp(buffer,"+ GAMEOVER\n") == 0) {
            printf("\nDas Spiel ist zu Ende. Es gibt keinen Gewinner.\n");
        } else {
            int playerNumber;
            char playerName[BUFFR];
            sscanf(buffer, "%*s %*s %d %s", &playerNumber, playerName);
            printf("\nDas Spiel ist zu Ende. Der Gewinner ist: %d - %s\n", playerNumber, playerName);
            //Return 2, fuer Spiel beendet. Looper gibt dann EXIT_SUCCESS(1) zurueck.
        }
        return 2;
    }

    // Ueberprueft ob der Server einfach nur die Verbindung beenden will
    if (strstr(buffer, "+ QUIT")!= NULL) {
        printf("\nDas Spiel ist zu Ende.\n");
        return 2;
    }

    /* BEISPIELOUTPUT SERVER START        BEISPIELOUTPUT VOM SERVER WAEHREND SPIEL   BEISPIELOUTPUT VOM SERVER SPIEL ENDE
     + MOVE 3000                           + NEXT 5                                  + GAMEOVER 2 Player 2
     + NEXT 8                              + FIELD 4,4                               + NEXT 5
     + FIELD 4,4                           + 4 12 4 15 11                            + FIELD 4,4
     + 4 12 4 15 *                         + 3 3 10 8 *                              + 4 12 4 15 11
     + 3 3 10 * *                          + 2 9 2 * 1                               + 3 3 10 8 *
     + 2 9 * * *                           + 1 13 * 6 14                             + 2 9 2 * 1
     + 1 13 * * *                          + ENDFIELD                                + 1 13 * 6 14
     + ENDFIELD                                                                      + ENDFIELD
    */


    printf("\nFuer deinen Zug hast du %d ms und ",shm->thinkTime);
    printf("Stein %d ist zu setzen!\n\n",shm->StoneToPlace);
    printf("Unser momentanes Spielfeld. Groesse: %d x %d\n",shm->fieldX, shm->fieldY);

    //Wir kennen jetzt die Spielfeldgroesse => SHM-pf (Playing Field) dafuer reservieren und einhaengen (2x Groesse von fieldX wegen 4 Merkmalen pro Stein!)
    if (shm->pfID == 0) {
        shm->pfID = shmget(KEY, (sizeof(short)*(shm->fieldX)*(shm->fieldX)*(shm->fieldY)),IPC_CREAT | 0775 );
        if (shm->pfID < 1) {
            writelog(logdatei,AT);
            perror("KIND");
            return EXIT_FAILURE;
        }
    }


    shm->pf = shmat(shm->pfID, 0, 0);
    writelog(logdatei,AT); //pf einhaengen


    //Im Fehlerfall pointed pf auf -1
    if (*(shm->pf) ==  -1) {
        fprintf(stderr, "Fehler, pf-shm: %s\n", strerror(errno));
        writelog(logdatei,AT);
    }

        readGameField(buffer,shm);

    //Bereitet den naechsten Zug vor (sendet Signal an Vater-Thinker)
    sendReplyFormatted(sock, "THINKING");
            if (handleRecv(sock,buffer) != 0)
        {
            return EXIT_FAILURE;
        }


    if (strcmp(buffer, "+ OKTHINK\n") != 0) {
              if (handleRecv(sock,buffer) != 0)
        {
            return EXIT_FAILURE;
        }

    }

    // Naechsten Spielzug ausdenken (ueber thinker)
    shm->thinking = 1;
    shm->pleaseThink = 1;
    // Sagt dem Vater, dass er jetzt denken soll
    kill(shm->pidDad, SIGUSR1);
    return EXIT_SUCCESS;
}
