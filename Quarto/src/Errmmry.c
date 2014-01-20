#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "Errmmry.h"
#include <time.h>

// Anzahl der zugewiesenen Speicherbereiche
int countint = 0;
int countchar = 0;

/**
 * Fuegt den Pointer auf den spaeter zu befreienden Bereich in ein Array hinzu
 *
 * @param adr der Pointer auf den Speicherbereich
 * @return 0 / -1
 */
int addint(int* intadr) {
	if (intadr == NULL ) {
		perror("\nFehler bei Malloc eines int pointers! \n");
		freeall();
		exit(EXIT_FAILURE);
	}

	varadr_int[countint] = intadr;
	countint++;
	return EXIT_SUCCESS;
}

int addchar(char* charadr) {
	if (charadr == NULL ) {
		perror("\nFehler bei Malloc eines char pointers! \n");
		freeall();
		exit(EXIT_FAILURE);
	}

	varadr_char[countchar] = charadr;
	countchar++;
	return EXIT_SUCCESS;
}

/**
 * Befreit alle in das Array bisher aufgenommenen Speicherbereiche
 *
 * @return 0
 */
int freeall() {
	int i;

	for (i = 0; i <= countint; i++) {
		free(varadr_int[i]);
	}

	for (i = 0; i <= countchar; i++) {
		free(varadr_char[i]);
	}

	return EXIT_SUCCESS;
}

/**
 * Schreibt eine Fehlermeldung mit Timestamp in die Datei log.txt
 * Um Zeilenzahl genau zu halten, bitte neben Syscalls aufrufen
 *
 * @param Filepointer zur log Datei, String im Format: "Dateipfad:Zeilennummer"
 * @return 0
 */
int writelog(FILE *filename, const char *location) {
	time_t result = time(NULL );
	char *t = ctime(&result);
	t[strlen(t) - 1] = 0;

	if (errno != 0) {
		printf("Ein Fehler wurde in der Logdatei dokumentiert. \n");
	}

	fprintf(filename, "%s --> ", t);
	fputs(strerror(errno), filename);
	fprintf(filename, " at %s \n", location);
	return EXIT_SUCCESS;
}
