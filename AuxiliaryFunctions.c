#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "SharedVariables.h"

/**
 * Haengt \n an den String fuer den Server an
 *
 * @param Socket, Pointer auf String wo das \n dran soll
 * @return Pointer auf String mit angehaengtem \n
 */
void sendReplyFormatted(int sock, char* reply) {
	char * container;
	container = malloc(sizeof(char) * (strlen(reply) + 2));
	strcpy(container, reply);
	strcat(container, "\n");
	send(sock, container, strlen(container), 0);
	free(container);
}

/**
 * Umgedrehte strcat Funktion um custom strings zu uebergeben, die fuer die korrekte uebertragung noetig sind
 *
 * @param Pointer
 * @return 0
 */
int antistrcat(char* dest, char* src, char* temp) {
	strcpy(temp, src);
	strcat(temp, dest);
	return EXIT_SUCCESS;
}

/**
 * readGameField bekommt folgendes uebergeben und liesst es in den pf-SHM ein
 * + 4 * * * *
 * + 3 * * * *
 * + 2 * * * *
 * + 1 * * * *
 * + ENDFIELD
 * Zeile 51: buffer2 wird mit strtok veraendert, fuer free wird eine Kopie benoetigt
 *
 * @param Puffer, SHM
 * @return 0 (Eingelesenes Spielfeld in *pf)
 */
int readGameField(char *buffer, sharedmem * shm) {
	char* buffer2;
	buffer2 = malloc(sizeof(char) * 128);
	char *buffer2temp = buffer2;
	int i = 0, znr = 0;
	char tmp = '*';
	buffer2 = strstr(buffer, "+ FIELD");
	buffer2 = strtok(buffer2, "\n");
	buffer2 = strtok(NULL, "\n");

	while (strcmp(buffer2, "+ ENDFIELD") != 0) {
		sscanf(buffer2, "%*s %d %[0-9* ]", &znr, buffer2);

		for (i = 0; i < shm->fieldX; i++) {
			*(shm->pf + i + ((znr - 4) * (-1)) * shm->fieldX) = -1;
			if (buffer2 == strchr(buffer2, tmp)) {
				sscanf(buffer2, "%*s %[0-9* ]", buffer2);
			} else {
				sscanf(buffer2, "%d %[0-9* ]", (shm->pf + i + ((znr - 4) * (-1)) * (shm->fieldY)), buffer2);
			}
		}
		buffer2 = strtok(NULL, "\n");
	}

	free(buffer2temp);
	return EXIT_SUCCESS;
}

/**
 * Gibt Spielfeld aus
 *
 * @param Pointer auf SHM
 * @return 0 (Spielfeld auf Konsole)
 */
int printGameField(sharedmem * shm) {
	int i, j;

	for (i = 0; i < shm->fieldY; i++) {
		printf("\n %d: ", i + 1);
		for (j = 0; j < shm->fieldX; j++) {
			if (*(shm->pf + j + i * shm->fieldY) == -1) {
				printf(" *");
			} else {
				printf(" %d ", *(shm->pf + j + i * shm->fieldY));
			}
		}
	}

	return EXIT_SUCCESS;
}
