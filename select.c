#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/shm.h>
#include "sharedVariables.h"
#include "errmmry.h"
#include "auxiliaryFunctions.h"

#define BUFFR 512
int doMove1(int sock, char* buffer,sharedmem * shm) {
	int size;
	printf("\nDoMove1: %s\n", buffer);

	sendReplyFormatted(sock, "THINKING");
	size = recv(sock, buffer, BUFFR - 1, 0);
	if (size > 0)
		buffer[size] = '\0';
	if (strcmp(buffer, "+ OKTHINK\n") != 0)
		size = recv(sock, buffer, BUFFR - 1, 0);
	printf("\nDoMove2: %s\n", buffer);

	// Naechsten Spielzug ausdenken (ueber thinker)
	shm->thinking = 1;
	shm->pleaseThink = 1;
	// Sagt dem Vater, dass er jetzt denken soll
	kill(shm->pidDad, SIGUSR1);
return EXIT_SUCCESS;

}
int doMove2(int sock, char* buffer) {
	int size;

	char* reply = malloc(sizeof(char)*15);
	size= read(fd[0], reply, 15);
	if (size < 0)
		perror("Fehler bei  read");
	printf("\nReplyDoMove: %s\n", reply);
	sendReplyFormatted(sock, reply);
	free(reply);


	size = recv(sock, buffer, BUFFR - 1, 0);
	if (size > 0)
		buffer[size] = '\0';
	if (strcmp(buffer, "+ MOVEOK\n") == 0) {

		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;

}

/* This function calls select to wait for data to read from */
/* one of the sockets passed as a parameter.                */
/* If more than timeout.tv_sec seconds elapses,it returns   */
/* EXIT_FAILURE hopefully due to game over                  */
/*                                                          */
/* @param                                                   */
/* @return on success, 0 on failure 1                       */
int waitforfds(int sock,char* buffer,sharedmem * shm) // nur ein Vorschlag der Parametrisierung, bitte feststellen,

                                               //was tatsaechlich benoetigt wird
{
   // Vorbereitungen fuer select() siehe man select
   int rc;
   int pipe=fd[0];
   fd_set fds;
   struct timeval timeout;
   /* Set time limit. */
   timeout.tv_sec = 10; // hier 10 Sekunden, kann beliebig angepasst werden!
   timeout.tv_usec = 0;
   /* Create a descriptor set containing our two filedescriptors (Socket and Pipe).  */
   FD_ZERO(&fds);
   FD_SET(sock, &fds);
   FD_SET(pipe, &fds);

   int status = checkServerReply(sock, buffer, shm);
	int moveStatus;
	int size;

   do {

      /* Hier soll der restliche Teil des Protokolls (Serverkommunikation, siehe looper) in einer Schleife geschehen */
doMove1(sock, buffer, shm);

         // select wartet auf eine Aenderung in einem der beiden Filedescriptoren (Pipe oder Socket) und gibt die Anzahl (int rc)
         // der filedescriptoren mit Aenderungen zurueck.
         rc = select(sizeof(fds)*8, &fds, NULL, NULL, &timeout);
         if (rc==-1) {
         perror("Error, select failed! \n");
         return EXIT_FAILURE;
         }

         if (rc > 0)
         {
            // Fall das Socket ready to read ist:
            if (FD_ISSET(sock, &fds)) {
            // Hier folgt das Handling fur die Antwort des Servers...

            }
            // Fall das Pipe ready to read ist:
            else if (FD_ISSET(pipe, &fds)) {
            // Hier folgt Handling fuer das Senden der Antwort an den Server (Read aus Pipe und move senden usw).
moveStatus = doMove2(sock, buffer);

 	size = recv(sock, buffer, BUFFR - 1, 0); writelog(logdatei,AT);
			printf("\nBuffer für Playtime: %s\n", buffer);

		if (size > 0)			buffer[size] = '\0';
		if (moveStatus != EXIT_FAILURE) {
			status = checkServerReply(sock, buffer, shm);
			if (status != 0) {
                break;
			}
		}

		 else {

			break;
		}
            }
         // falls select 0 returned heisst die Wartezeit ist ohne Ereigniss abgelaufen
         } else perror("Select timed out, possibly due to game being over. \n");
      } while (rc!=0);
      // solange es zu keinem Timeout kommt, soll die Kommunikation in der Schleife weiterlaufen.
   return EXIT_SUCCESS; // Falls die Schleife ohne Probleme zu Ende durchlaeuft
}


