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
#include "select.h"
#include "Playtime.h"

#define BUFFR 512

/**
 * Short one line description.                           
 * 
 * Longer description. If there were any, it would be    
 * here.
 * <p>
 * And even more explanations to follow in consecutive
 * paragraphs separated by HTML paragraph breaks.
 *
 * @param  variable Description text text text.          
 * @return Description text text text.
 */
int doMove(int sock, char* buffer)
{
  int size;
  char* reply = malloc(sizeof(char)*15);
  size= read(fd[0], reply, 15);
  if (size < 0) perror("Fehler bei read");
  printf("\nReplyDoMove: %s\n", reply);
  sendReplyFormatted(sock, reply);
  free(reply);
	
  size = recv(sock, buffer, BUFFR - 1, 0);
  if (size > 0) buffer[size] = '\0';
  if (strcmp(buffer, "+ MOVEOK\n") == 0)  printf("Zug wurde akzeptiert!\n");
	else return EXIT_FAILURE;
		
	return EXIT_SUCCESS;
}

/**
* Diese Funktion implementiert select() und wechselt zwischen der Berechnung des naechsten
* Zugs (Vater-Thinker) und der fortfuehrung der Kommunikation mit dem Server (Kind-Connector)
*
* @param Socket fuer Kommunikation mit Server, buffer fuer auslesen der Antwort, Shared Memory
* @return on success, 0 on failure 1
**/
int waitforfds(int sock,char* buffer,sharedmem * shm) // nur ein Vorschlag der Parametrisierung, bitte feststellen,

{
  // Vorbereitungen fuer select() siehe man select
  int rc,size,status,biggest;
  int pipe=fd[0];
  fd_set fds;
  struct timeval timeout;

  if (pipe>sock) biggest=pipe;
  else biggest=sock;

	
  /*
  * checkServerReply muss leider einmal ausserhalb der Schleife ausgefuehrt werden, da die Antwort des Servers schon frueher im Code 
  * vom Socket gelesen wurde und hier im buffer uebergeben wurde
  */
  checkServerReply(sock, buffer, shm);

  do 
  {	
 	/* Unsere 2 Filedescriptoren werden hinzugefuegt. (Socket und Pipe). */
   	FD_ZERO(&fds);
   	FD_SET(sock, &fds);
   	FD_SET(pipe, &fds);

	 /* Zeitlimit fuer select wird eingestellt */
   	timeout.tv_sec = 10; // hier 10 Sekunden, kann beliebig angepasst werden!
   	timeout.tv_usec = 0;
    
    /*
    * Select laesst den Prozess schlafen und beobachtet fuer eine in timeout spezifizierte Zeit lang die in fds hinzugefuegten 
    * Filedescriptoren (in unserem Falle Socket und Pipe).
    * Falls waehrenddessen ein Filedescriptor ready to read wird, weckt select den Prozess auf und gibt die Anzahl der ready to read
    * Filedescriptoren zurueck. Sollte  
    */
    rc = select(biggest+1, &fds, NULL, NULL, &timeout);
    if (rc==-1) 
    {
      perror("Error, select failed! \n");
      return EXIT_FAILURE;
    }

    if (rc > 0)
    {	
      printf("SELECT: Number of FDs ready: %d \n",rc);
      // Fall das Socket ready to read ist:
      if (FD_ISSET(sock, &fds)) 
      {
		    printf("SELECT: Socket is ready to read!\n");
        size = recv(sock, buffer, BUFFR - 1, 0); writelog(logdatei,AT);
        printf("\nBuffer fuer Playtime: %s\n", buffer);
				if (size > 0)                        buffer[size] = '\0';
        status = checkServerReply(sock, buffer, shm);
        if (status != 0) break;
      }
            // Fall das Pipe ready to read ist:
      else if (FD_ISSET(pipe, &fds)) 
      {
		    printf("SELECT: Pipe is ready to read!\n");
        doMove(sock, buffer);
      }
         // falls select 0 returned heisst die Wartezeit ist ohne Ereigniss abgelaufen
    } else if (rc==0) perror("Select timed out!. \n");
  } while (rc!=0);
    // solange es zu keinem Timeout kommt, soll die Kommunikation in der Schleife weiterlaufen.
   return EXIT_SUCCESS; // Falls die Schleife ohne Probleme zu Ende durchlaeuft
}
