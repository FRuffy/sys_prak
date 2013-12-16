#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "sharedVariables.h"


/* Hängt /n an den String für den Server an */
void sendReplyFormatted(int sock, char* reply)
{
    char * container;
    container = malloc(sizeof(char)*(strlen(reply)+2));
    strcpy(container,reply);
    strcat(container, "\n");
    send(sock,container,strlen(container),0);
    free(container);
}

/* Eine simple umgedrehte strcat Funktion um custom strings zu uebergeben, die fuer die korrekte Übertragung noetig sind */
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

int readGameField(char *buffer,sharedmem * shm)
{
    char* buffer2;
    buffer2 = malloc(sizeof(char)*128);
    int i=0, znr=0;
    char tmp= '*';
    buffer2 = strstr(buffer,"+ FIELD");
    buffer2 = strtok( buffer2, "\n" );
    buffer2 = strtok( NULL , "\n" );

    while (strcmp(buffer2,"+ ENDFIELD") != 0)
    {
    	//Folgende 4 Kommentarzeichen entfernen um zu kapieren wie die Funktion funktioniert!
    	//	    printf("\n%s\n", buffer2);
    		    sscanf (buffer2,"%*s %d %[0-9* ]", &znr, buffer2);

    		    for (i=0; i<shm->fieldX; i++) {
    		    	*(shm->pf+i+(znr-1)*shm->fieldX) = -1;
    	//	    	printf("Buffer2: %s\n", buffer2);
    		    	if (buffer2 == strchr(buffer2, tmp)) {
    	//	    		printf("X=> *\n");
    		    		sscanf(buffer2,"%*s %[0-9* ]", buffer2);
    		    	}
    		    	else {
    		    		sscanf (buffer2,"%d %[0-9* ]",(shm->pf+i+(znr-1)*(shm->fieldY)), buffer2);
    	//	    		printf(" => %d \n", *(pf+i+(znr-1)*shm->fieldY));
    		    	}
    		    }
    		    buffer2 = strtok( NULL, "\n" );
    	        }
    	        free(buffer2);
    		return EXIT_SUCCESS;
    	}

/*
* printGameField gibt aktuellen Zustand des Spielfeldes aus
*/

int printGameField(sharedmem * shm)
{
    int i,j;
    for (i=shm->fieldY-1; i>=0; i--)
    {
        printf("\n %d: ", i+1);
        for (j=0; j<shm->fieldX; j++)
        {
            if (*(shm->pf+j+i*shm->fieldY)==-1)
            {
                printf(" *");
            }
            else
            {
                printf(" %d", *(shm->pf+j+i*shm->fieldY));
            }
        }
    }
    printf("\n     A B C D\n");
    return EXIT_SUCCESS;
}
