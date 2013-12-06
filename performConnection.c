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

/*Formatierte Ausgabe an den Server, \n wird benoetigt um das Ende der Uebertragung zu signalisieren */
void sendReplyFormatted(int sock, char* reply)
{
    char * container;
    container = malloc(sizeof(char)*(strlen(reply)+2));
    strcpy(container,reply);
    strcat(container, "\n");
    send(sock,container,strlen(container),0);
    free(container);
}

/* Eine simple umgedrehte strcat Funktion um custom strings zu übergeben, die für die korrekte Übertragung nötig sind  */
char* antistrcat(char* dest, char* src)
{
    char * container;
    container = malloc(sizeof(char)*(strlen(dest)+strlen(src)+1));
    strcpy(container,src);
    strcat(container, dest);

    return container;
}

 /**
 * readGameField bekommt folgendes uebergeben und liesst es in den pf-SHM ein
 * + 4 * * 15 *
 * + 3 * * * *
 * + 2 * * * *
 * + 1 * * * *
 * + ENDFIELD
 */
int readGameField(char *buffer2) {
int i=0, znr=0;

while (strcmp(buffer2,"+ ENDFIELD") != 0)
        {
//Kommentarzeichen entfernen um zu kapieren wie die Funktion funktioniert!
//	    printf("\n%s\n", buffer2);
	    sscanf (buffer2,"%*s %d %[0-9* ]", &znr, buffer2);

	    for (i=0; i<=shm->fieldX; i++) {
	    	*(pf+i+(znr-1)*shm->fieldX) = -1;
	    		sscanf (buffer2,"%d %[0-9* ]",(pf+i+(znr-1)*shm->fieldY), buffer2);
//	    		printf("=>%d \n", *(pf+i+(znr-1)*shm->fieldY));
	    		sscanf (buffer2,"%*s %[0-9* ]",buffer2);
//	    		printf(": %s \n", buffer2);
	    }
	    buffer2 = strtok( NULL, "\n" );
        }
	return EXIT_SUCCESS;
}

/**
* printGameField gibt aktuellen Zustand des Spielfeldes aus
*/
int printGameField() {
	int i,j;
	for (i=shm->fieldY-1; i>=0; i--) {
		printf("\n %d: ", i+1);
		for (j=0; j<shm->fieldX; j++) {
			if (*(pf+j+i*shm->fieldY)==-1) {
				printf(" *");
			}
			else {
				printf(" %d", *(pf+j+i*shm->fieldY));
			}
		}
	}
	return EXIT_SUCCESS;
}

int performConnection(int sock)
{
    int size, i=0;//size zur Fehlerbehandlung für recv
    char* reader;
    char* temp;
    reader = malloc(sizeof(char)*20);
    char* buffer =  malloc(sizeof(char)*BUFFR);
    char* buffer2 =  malloc(sizeof(buffer));
    temp = malloc(sizeof(char)*30);
    //strcpy(gameID,"ID  fm1y4PiVKfU");
    //strcpy(gameID,"ID  528902d1b0074");

    /* Teil 1: Lese die Client-Version des Servers und antworte mit eigener (formatierten) Version
     Behandle die Antwort des Servers */

    size = recv(sock, buffer, BUFFR-1, 0);
    sscanf(buffer, "%*s%*s%*s%s", reader);
    if (size > 0) buffer[size]='\0';
    printf("\nDie Version des Servers ist: %s\n", reader);

    temp = antistrcat(conf->version,"VERSION ");
    sendReplyFormatted(sock,temp);
    free(temp);

    size = recv(sock, buffer, BUFFR-1, 0);
    if (size > 0) buffer[size]='\0';

    if (buffer[0] == '-')
    {
        printf("\nDer Server akzeptiert die Version %s dieses Clients nicht!\n",conf->version);
        free(buffer);
        free(reader);
        close(sock);
        return EXIT_FAILURE;
    }
    else
    {
        printf("\nDie Client-Version wurde akzeptiert, uebertrage Spiel-ID...\n");
    }

    /* Teil 2: Der Server erwartet Game-ID, schicke diese und behandle Antwort.
    Falls die ID fehlerhart oder das Spiel des Servers nicht Quarto ist wird die Verbindung beendet*/

    temp = antistrcat(shm->gameID,"ID ");
    sendReplyFormatted(sock, temp);
    free(temp);
    size = recv(sock, buffer, BUFFR-1, 0);
    if (size > 0) buffer[size]='\0';
    sscanf(buffer, "%*s%*s%s", reader);

    if (buffer[0] == '-')
    {
        printf("\nDie uebergebene Game-ID fehlt oder ist fehlerhaft! Beende Verbindung\n");
        free(buffer);
        free(reader);
        close(sock);
        return EXIT_FAILURE;
    }
    else if (strcmp(reader,"Quarto") !=0)
    {
        printf("\nDas Spiel, das der Server spielt, ist nicht Quarto! Beende Verbindung.\n");
        free(buffer);
        free(reader);
        close(sock);
        return EXIT_FAILURE;
    }
    else
    {
        printf("\nDer Server moechte %s spielen. Und wir auch!\n", reader);
    }
    /* Teil 3.1: Hatten wir mit unserer ID Erfolg erfahren wir zunächst den Namen des Spiels
       und senden die ggf. übergebene Spielernummer.
       Anschließend wird die Antwort des Servers auf die Nummer behandelt und Name und Nummer des Spielerplatzes ausgelesen
    */

    size = recv(sock, buffer, BUFFR-1, 0);
    if (size > 0) buffer[size]='\0';

    sscanf(buffer, "%*s%s", shm->gameName);
    printf("\nSpiel: %s\n", shm->gameName); //Zeige Spielnamen an, schneide das "+" ab
    temp = antistrcat(conf -> playernumber,"PLAYER ");
    sendReplyFormatted(sock,temp);
    free(temp);
    size = recv(sock, buffer, BUFFR-1, 0);
    if (size > 0) buffer[size]='\0';


    if (buffer[0] == '-')
    {
        if (buffer[2] == '-')
        {
            printf("\nEs wurde eine ungueltige Spielernummer eingegeben! Beende Verbindung\n");
        }
        else
        {
            printf("\nAlle Plaetze sind bereits belegt, versuchen sie es spaeter noch einmal!\n");
            //ACHTUNG. Diese Meldung erscheint auch, wenn man in der client.conf eine Spielernummer an gibt, die beim Erstellen des Spiels einem Computerspieler zugeordnet wurde!
        }
        free(buffer);
        free(reader);
        close(sock);
        return EXIT_FAILURE;
    }
    else
    {
        sscanf(buffer, "%*s %*s %d %s",&(shm->player[0].playerNumber), &(shm->player[0].playerName));
        //printf("\nDu spielst mit dem Namen %s, deine Nummer ist %d\n", shm->player[0].playerName,shm->player[0].playerNumber);
    }
    /* Teil 3.2: Hier wird der Übergang in die Spielverlaufsphase eingeleitet.
       Der Server sendet uns die Namen unseres Gegenspielers und des Spielernummer, wobei überprüft wird ob ein Spieler bereits verbunden ist.
    */
    size = recv(sock, buffer, BUFFR-1, 0);
    if (size > 0) buffer[size]='\0';

    printf("Folgenes wird jetzt verarbeitet:\n");
    printf("=====================================");
    if (size > 0) buffer[size]='\0';
    printf("%s\n",buffer);
    printf("=====================================\n");

    if (buffer[0] == '-') {
    	printf("\nFehler bei Uebermittlung der Spielparameter!\n");
    	return EXIT_FAILURE;
    }
    buffer2 = strtok( buffer, "\n" );

    //Verarbeite: "+ TOTAL 2" (Anzahl der Spieler)
    sscanf(buffer2, "%*s %*s %i", &(shm->playerCount));
    if (shm->playerCount > 8) {
    	printf("\nDiese Version des Clients unterstuetzt maximal 8 Spieler!\n");
    	return EXIT_FAILURE;
    }

    //Verarbeite: "+ 1 GEGENSPIELERNAME 0" (Uebermittlung der Gegenspieler
    //(Nummer, Name, Flag ob bereit) solange bis Server "+ ENDPLAYERS" sendet)
    buffer2 = strtok( NULL, "\n" );
    while (strcmp(buffer2,"+ ENDPLAYERS") != 0)
        {
        sscanf(buffer2, "%*s %d %s %i", &shm->player[i].playerNumber, &shm->player[i].playerName, &shm->player[i].playerReady);
        buffer2 = strtok( NULL, "\n" );
        i++;
        }
    printf("\nEs spielen %i Spieler, naemlich:\n", shm->playerCount);
    printf("Spielernummer\tSpielername\tBereit\n");
    for (i=0; i<shm->playerCount; i++) {
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
    printf("Spielfeldgroesse: %d x %d\n",shm->fieldX, shm->fieldY);

    //Wir kennen jetzt die Spielfeldgroesse => SHM-pf (Playing Field) dafuer reservieren und einhaengen (2x Groesse von fieldX wegen 4 Merkmalen pro Stein!)
    pfID = shmget(IPC_PRIVATE, (sizeof(short)*(shm->fieldX)*(shm->fieldX)*(shm->fieldY)), IPC_CREAT | IPC_EXCL | 0775);
    if (pfID < 1)
    {
        printf("Error: No pf-SHM");
        return EXIT_FAILURE;
    }
    pf = shmat(pfID, 0, 0); //pf einhaengen
    if (pf == (void *) -1)   //Im Fehlerfall pointed pf auf -1
    {
        fprintf(stderr, "Fehler, pf-shm: %s\n", strerror(errno));
        writelog(logdatei,AT);
    }
    buffer2 = strtok( NULL, "\n" );


    readGameField(buffer2);

    printGameField();


    //WAIT <->OKWAIT Schleife, später zu implementieren.
      do
        {
            send(sock,"OKWAIT\n",strlen("OKWAIT\n"),0);
            size = recv(sock, buffer, BUFFR-1, 0);
            if (size > 0) buffer[size]='\0';
            printf("\nServer bittet zu warten.\n");
            printf("\n%s",buffer);

        }
        while (strcmp(buffer,"+ WAIT\n") == 0);
    free(buffer);
    free(reader);
    close(sock);
    shmdt(pf);
    shmdt(shm);
    return EXIT_SUCCESS;
}
