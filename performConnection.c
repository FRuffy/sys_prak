#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "sharedVariables.h"
#include <errno.h>  //fuer Fehlerbehandlung (Harun)

#define BUFFR 256
//#define GAMEKINDNAME "Quarto"
//#define VERSION "VERSION 1.0"
//Importiere Variablen

//Prüfe ob der Gegenspieler im Spiel und bereit ist
/*
int checkPlayerReady(char* response)
{
    if(strstr(response,"0") !=NULL)
    {
        printf("\nDein Gegner ist noch nicht bereit, bitte warte ein wenig.\n");
        return 1;
    }
    else
    {
        printf("\nDein Gegenspieler ist bereit");
        return 0;
    }
}
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
//Bringe die zu sendende Antwort in ein für den Server angepasstes Format.
int sendReplyFormatted(int sock, char* reply)
{
    char * container;
    container = malloc(sizeof(char)*strlen(reply)*2); // Container-String der die Antwort übernimmt und eine Newline angehängt bekommt.
    int err;
    strcpy(container,reply);
    strcat(container, "\n");
    err = send(sock,container,strlen(container),0); //Sende den Container an den Server, err für Fehlerbehandlung.
    free(container);
    return err;
}



int performConnection(int sock)
{

    char* buffer =  malloc(sizeof(char)*BUFFR); //Initialisiere den Buffer für die Kommunikation
    //strcpy(gameID,"ID  fm1y4PiVKfU");
    //strcpy(gameID,"ID  528902d1b0074");

    //Initialisiere Hilfsvariablen für die formatierte Ausgabe einer Serverantwort
    char* reader;
    int readInt;
    reader = malloc(sizeof(char)*20);
    //readInt = malloc(sizeof(char));
    int var1;
    int err; //Return-Wert von err für Fehlerbehandlung

    err = recv(sock, buffer, BUFFR-1, 0); //Empfange Server Nachrichtung und speichere sie in Buffer
    sscanf(buffer, "%*s%*s%*s%s", reader);
    printf("\nDie Version des Servers ist: %s\n", reader);
    //printf("\n%s",buffer);

    if (err > 0) buffer[err]='\0'; //Markiere Ende der Übertragung mit \0
    //printf("\n%s",buffer); //Ausgabe
    char* temp;
    temp = malloc(sizeof(char)*(strlen("VERSION ")+10));
    strcpy(temp,"VERSION ");

    strcat(temp,conf->version);
    err = sendReplyFormatted(sock,temp);
    free(temp);
    //send(sock,VERSION,strlen(VERSION),0); //Schicke Version des Spieles an Server

    err = recv(sock, buffer, BUFFR-1, 0);
    if (err > 0) buffer[err]='\0';
    //printf("\n%s",buffer);

    if (buffer[0] == '-')
    {
        printf("\nDer Server akzeptiert die Version %s dieses Clients nicht!\n",conf->version);
        free(buffer);
        free(reader);
        // free(readInt);
        close(sock);
        return EXIT_FAILURE;
    }
    else
    {
        printf("\nDie Client-Version wurde akzeptiert, uebertrage Spiel-ID...\n");
    }

    // Durch Umbau der sendReplyFormatted, sodass sie Values und keine References entgegen nimmt koennte man sich einiges sparen
    char *ID=NULL; //Die modifizierte GameID die wir mit der performConnection Funktion teilen werden
    ID = malloc(sizeof(char)*13);
    strcpy(ID,"ID ");
    strcat(ID, shm->gameID);

    err = sendReplyFormatted(sock, ID); //Schicke Spiel-ID an Server
    err = recv(sock, buffer, BUFFR-1, 0);
    if (err > 0) buffer[err]='\0';
    //printf("\n%s",buffer);
    sscanf(buffer, "%*s%*s%s", reader); //Scanne Antwort nach relevanten Variablen.

    if (strcmp(reader,"Quarto") !=0)   //Teste ob auch das richtige Spiel vom Server gespielt wird, falls nicht beende Verbindung
    {
        printf("Das Spiel, das der Server spielt, ist nicht Quarto! Beende Verbindung.");
        // printf("\n%s",buffer);
        free(buffer);
        free(reader);
        //  free(readInt);
        close(sock);
        return EXIT_FAILURE;
    }
    else
    {
        printf("\nDer Server moechte %s spielen. Und wir auch!\n", reader);
    }

//Schicke Spieler Konfiguration und lese Nachrichten vom Server
    err = recv(sock, buffer, BUFFR-1, 0);
    if (err > 0) buffer[err]='\0';
    printf("\nSpiel: %s",&buffer[2]); //Zeige Spielnamen an, schneide das "+" ab
    err = sendReplyFormatted(sock,playerNum);
    // err =  send(sock,"PLAYER\n",strlen("PLAYER\n"),0);
    err = recv(sock, buffer, BUFFR-1, 0);
    if (err > 0) buffer[err]='\0';

//Teste ob überhaupt noch was frei ist, falls ja gebe Name des Spielers und Nummer aus, falls nicht beende Übertragung
    if (buffer[0] == '-')
    {
        printf("\nAlle Plaetze sind bereits belegt, versuchen sie es spaeter noch einmal!\n");
        free(buffer);
        free(reader);
        //free(readInt);
        close(sock);
        return EXIT_FAILURE;
    }
    else
    {
        sscanf(buffer, "%*s %*s %d %s",&readInt, reader);
        printf("\nDu spielst mit dem Namen %s, deine Nummer ist %d\n", reader,readInt);
    }

    err = recv(sock, buffer, BUFFR-1, 0);
    if (err > 0) buffer[err]='\0';

    sscanf(buffer, "%*s %*s %i", &var1); //Scanne Anzahl der Spieler, normalerweise immer 2
    printf("\nEs spielen %i Spieler.", var1);
    // printf("\n%s",buffer);
    sscanf(buffer, "%*[^\n]%*s %d %s %i", &readInt,reader,&var1); //Scanne Namen des Spielers und überprüfe ob dieser Bereit ist.
    if (var1 == 0)
    {
        printf("\nSpieler %s mit der Nummer %d ist noch nicht bereit.\n",reader,readInt );
    }
    else
    {
        printf("\nSpieler %s mit der Nummer %d ist bereit!\n",reader,readInt );
    }

    //WAIT <->OKWAIT Schleife, später zu implementieren.
    /*     do
        {
            send(sock,"OKWAIT\n",strlen("OKWAIT\n"),0);
            err = recv(sock, buffer, BUFFR-1, 0);
            if (err > 0) buffer[err]='\0';
            printf("\nServer bittet zu warten.\n");

        }
        while (strcmp(buffer,"+ WAIT\n") == 0); */

    free(buffer);
    free(reader);
    // free(readInt);
    close(sock);
    return EXIT_SUCCESS;
}




