#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "errmmry.h"
#include <time.h>

int varcount=0; // Anzahl der zugewiesenen Speicherbereiche

/**
 * Fuegt den Pointer auf den spaeter zu befreienden Bereich in ein Array hinzu.
 *
 * @param  adr Der Pointer auf den Speicherbereich
 * @return Gibt 0 zurueck.
 */
int addfree(int* adr){
varadresses[varcount]=adr;
varcount++;
return EXIT_SUCCESS;        
}
/**
 * Befreit alle in das Array bisher aufgenommenen Speicherbereiche.
 *
 * @return Gibt 0 zurueck.
 */
int freeall(){
int i;
for(i=0;i<=varcount;i++){
        free(varadresses[i]);
}
printf("Es wurden %d Variablen befreit \n",varcount);
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
int writelog(FILE *filename,const char *location){
time_t result = time(NULL);
char *t = ctime(&result);
t[strlen(t)-1] = 0;
if (errno!=0) printf("Ein Fehler wurde in der Logdatei dokumentiert. \n");
fprintf(filename,"%s --> ",t);
fputs(strerror(errno),filename);
fprintf(filename," at %s \n",location);
return EXIT_SUCCESS;
}
