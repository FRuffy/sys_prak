#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "Errmmry.h"
#include <time.h>

int countint=0; // Anzahl der zugewiesenen Speicherbereiche
int countchar=0;



/**
 * Fuegt den Pointer auf den spaeter zu befreienden Bereich in ein Array hinzu.
 *
 * @param  adr Der Pointer auf den Speicherbereich
 * @return Gibt 0 zurueck.
 */



int addint(int* intadr) {

        if (intadr==NULL){
                perror("Fehler bei Malloc eines int pointers! \n");
                exit(EXIT_FAILURE);
        }

        varadr_int[countint]=intadr;
        countint++;
                printf("\n INTVariable %d alloziert!\n",countint);

        return EXIT_SUCCESS;
}


int addchar(char* charadr) {

        if (charadr==NULL){
                perror("Fehler bei Malloc eines char pointers! \n");
               exit(EXIT_FAILURE);
        }

        varadr_char[countchar]=charadr;
       countchar++;
        printf("\n CHARVariable %d alloziert!\n",countchar);
        return EXIT_SUCCESS;
}


/**
 * Befreit alle in das Array bisher aufgenommenen Speicherbereiche.
 *
 * @return Gibt 0 zurueck.
 */
int freeall() {
        int i;

        for(i=0;i<=countint;i++) {
        free(varadr_int[i]);
        }

        for(i=0;i<=countchar;i++) {
                printf("\nICHB BEFREIFEEEEE\n");
        free(varadr_char[i]);
        }

        printf("Es wurden %d Variablen befreit \n",countint+countchar);
        return EXIT_SUCCESS;
}


/**
 * Schreibt eine Fehlermeldung mit Timestamp in die Datei log.txt
 * Um Zeilenzahl genau zu halten, bitte neben Syscalls aufrufen.
 *
 * @param Filepointer zur log Datei
 * @param String im Format: "Dateipfad:Zeilennummer".
 * @return Gibt 0 zurueck.
 */
int writelog(FILE *filename,const char *location) {
        time_t result = time(NULL);
        char *t = ctime(&result);
        t[strlen(t)-1] = 0;

        if (errno!=0) {
                printf("Ein Fehler wurde in der Logdatei dokumentiert. \n");
        }

        fprintf(filename,"%s --> ",t);
        fputs(strerror(errno),filename);
        fprintf(filename," at %s \n",location);
        return EXIT_SUCCESS;
}
