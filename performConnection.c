#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include "sharedVariables.h"
#include <errno.h>

#define BUFFR 256
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

/*

void testBuffer(char* buffer, int size)
{
    int i;
    for (i=0; i<size; i++)
    {
        if (buffer[i] == '\n') printf(" \\n ");
        else
            printf(" %c ",buffer[i]);
    }
}
*/

/*Formatierte Ausgabe an den Server, \n wird benötigt um das Ende der Übertragung zu signalisieren */

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

//printf("\n%s",buffer);

int performConnection(int sock)
{
    int readInt, readInt2, size; // readInt um Intwerte des Buffers zu scannen, size zur Fehlerbehandlung für recv
    char* reader;
    char* temp;
    reader = malloc(sizeof(char)*20);
    char* buffer =  malloc(sizeof(char)*BUFFR);
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
    printf("\nSpiel: %s",&buffer[2]); //Zeige Spielnamen an, schneide das "+" ab
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
        }
        free(buffer);
        free(reader);
        close(sock);
        return EXIT_FAILURE;
    }
    else
    {
        sscanf(buffer, "%*s %*s %d %s",&readInt, reader);
        printf("\nDu spielst mit dem Namen %s, deine Nummer ist %d\n", reader,readInt);
    }
    /* Teil 3.2: Hier wird der Übergang in die Spielverlaufsphase eingeleitet.
       Der Server sendet uns die Namen unseres Gegenspielers und des Spielernummer, wobei überprüft wird ob ein Spieler bereits verbunden ist.
    */
    size = recv(sock, buffer, BUFFR-1, 0);
    if (size > 0) buffer[size]='\0';
    sscanf(buffer, "%*s %*s %i", &readInt2);
    printf("\nEs spielen %i Spieler.", readInt2);
    sscanf(buffer, "%*[^\n]%*s %d %s %i", &readInt,reader,&readInt2);

    if (readInt2 == 0)
    {
        printf("\nSpieler %s mit der Nummer %d ist noch nicht bereit.\n",reader,readInt );
    }
    else
    {
        printf("\nSpieler %s mit der Nummer %d ist bereit!\n",reader,readInt );
    }


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
    return EXIT_SUCCESS;
}








