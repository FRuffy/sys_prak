#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "SharedVariables.h"

/**
 * Haengt /n an den String fuer den Server an
 *
 * @param  Socket, Pointer auf String wo das \n dran soll
 * @return Pointer auf String mit angehaengtem \n
 */
void sendReplyFormatted(int sock, char* reply) {
    char * container;
    container = malloc(sizeof(char)*(strlen(reply)+2));
    strcpy(container,reply);
    strcat(container, "\n");
    send(sock,container,strlen(container),0);
    free(container);
}

/**
 * umgedrehte strcat Funktion um custom strings zu uebergeben, die fuer die korrekte uebertragung noetig sind
 *
 * @param  Pointer
 * @return
 */
int antistrcat(char* dest, char* src, char* temp) {
    strcpy(temp,src);
    strcat(temp, dest);
    return EXIT_SUCCESS;
}

/**
 * readGameField bekommt folgendes uebergeben und liesst es in den pf-SHM ein
 * + 4 * * 15 *
 * + 3 * * * *
 * + 2 * * * *
 * + 1 * * * *
 * + ENDFIELD
 *
 * @param  Puffer, SHM
 * @return 0 (Eingelesenes Spielfeld in *pf)
 */
int readGameField(char *buffer,sharedmem * shm) {
    char* buffer2;
    buffer2 = malloc(sizeof(char)*128);
    char *buffer2temp = buffer2; //buffer2 wird mit strtok veraendert, fuer free wird eine Kopie benoetigt
    int i=0, znr=0;
    char tmp= '*';
    buffer2 = strstr(buffer,"+ FIELD");
    buffer2 = strtok( buffer2, "\n" );
    buffer2 = strtok( NULL , "\n" );

    while (strcmp(buffer2,"+ ENDFIELD") != 0) {
        //Folgende 4 Kommentarzeichen entfernen um zu kapieren wie die Funktion funktioniert!
        // printf("\n%s\n", buffer2);
        sscanf (buffer2,"%*s %d %[0-9* ]", &znr, buffer2);

        for (i=0; i<shm->fieldX; i++) {
            *(shm->pf+i+((znr-4)*(-1))*shm->fieldX) = -1;
            //printf("Buffer2: %s\n", buffer2);
            if (buffer2 == strchr(buffer2, tmp)) {
                //printf("X=> *\n");
                sscanf(buffer2,"%*s %[0-9* ]", buffer2);
            } else {
                sscanf (buffer2,"%d %[0-9* ]",(shm->pf+i+((znr-4)*(-1))*(shm->fieldY)), buffer2);
                //printf(" => %d \n", *(shm->pf+i+(znr-1)*shm->fieldY));
            }
        }
        buffer2 = strtok( NULL, "\n" );
    }

    free(buffer2temp);
    return EXIT_SUCCESS;
}

/**
 * Konvertiert in 4-stellige Binaerdarstellung
 */
int byte_to_binary(int n, char* pointer) {
   int c, d, count;
   //char *pointer;
   count = 0;
   //pointer = (char*)malloc(32+1);

   if (pointer == NULL) {
     return EXIT_FAILURE;
   }

   for (c=3; c>=0 ; c--) {
      d = n >> c;
      if ( d & 1 ) {
         *(pointer+count) = 1 + '0';
      } else {
         *(pointer+count) = 0 + '0';
      }
      count++;
   }

   *(pointer+count) = '\0';
   return  EXIT_SUCCESS;
}

/**
 * gibt Spielfeld aus
 *
 * @param  Pointer auf SHM
 * @return 0 (Spielfeld auf Konsole)
 */
int printGameField(sharedmem * shm) {
    int i, j;

    for (i=shm->fieldY-1; i>=0; i--) {
        printf("\n %d: ", i+1);
        for (j=0; j<shm->fieldX; j++) {
            if (*(shm->pf+j+i*shm->fieldY)==-1) {
                printf(" *");
            } else {
                printf(" %d ", *(shm->pf+j+i*shm->fieldY));
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * Gibt das Spielfeld speziell f��r ein Quarto 4x4 Spiel aus
 */
int printGameFieldQuarto4x4(sharedmem * shm, char* stones) {

    stones[0] = '\0';
    char* stone = malloc(sizeof(char)*5);

    int i,j;
    printf("\n+");

    for (i=shm->fieldY-1; i>=0; i--) {
        printf("-------");
    }

    printf("+");

    for (i=0; i<shm->fieldY; i++) {
        printf("\n+%29c", '+');
        printf("\n+ %d:", i+1);
        for (j=0; j<shm->fieldX; j++) {
            if (*(shm->pf+j+i*shm->fieldY)==-1) {
                strcat(stones,"****");
                printf(" **** ");
            } else {
                //strcpy(stone,byte_to_binary(*(shm->pf+j+i*shm->fieldY)));
               byte_to_binary(*(shm->pf+j+i*shm->fieldY),stone);
                strcat(stones,stone);
                printf(" %s ", stone);
            }
        }
        printf(" +");
    }

    printf("\n+%29c\n+", '+');

    for (i=shm->fieldY-1; i>=0; i--) {
        printf("-------");
    }

    printf("+");
    free(stone);
    return EXIT_SUCCESS;
}

