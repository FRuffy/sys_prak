#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "sharedVariables.h"
#include "errmmry.h"
#include "auxiliaryFunctions.h"




FILE *logdatei; // Die Logdatei, die Fehler bestimmter Systemfunktionen mitzeichnet und den Ort angibt


int main (int argc, char** argv )
{
    config_struct *conf; // Die Struktur, die die Konfigurationsparameter der Datei speichert

    logdatei =fopen("log.txt","w+");
    conf = calloc(5,sizeof(config_struct));
   struct sharedmem *shm = calloc(50,sizeof(sharedmem));
    /* Initialisierung der Shared Memory */
    int shmID;
    int shmSize = sizeof(struct sharedmem);
    // sharedMem Hilfe: http://www.nt.fh-koeln.de/vogt/bs/animationen/SharedMemAnimation.pdf

    shmID = shmget(IPC_PRIVATE, shmSize, IPC_CREAT | IPC_EXCL | 0775);

    if (shmID < 1)
    {
        printf("Error: No SHM");
        free(conf);
        fclose(logdatei);
        return EXIT_FAILURE;
    }

    shm = shmat(shmID, 0, 0); //SHM einhaengen ;

    if (shm == (void *) -1)   //Im Fehlerfall pointed shm auf -1
    {
        fprintf(stderr, "Fehler, shm: %s\n", strerror(errno));
        writelog(logdatei,AT);
    }

    pid_t pid = 0;
    pid = fork();

    /* Ab hier wird in 2 Prozesse, dem Think und dem Connector, aufgespalten */

    if ((pid) < 0)
    {
        fprintf(stderr, "Fehler bei fork(): %s\n", strerror(errno));
        writelog(logdatei,AT);

        shmctl(shmID,IPC_RMID, NULL);
        fclose(logdatei);
        free(conf);
        shmdt(shm);
        return EXIT_FAILURE;
    }
    else if (pid == 0)
    {
        //Kind - soll laut Spezifikation die Verbindung herstellen (performConnection() ausfuehren)
 initConnection(argc, argv,shm,conf);

    }
    else
    {
        //Elternprozess - soll laut Spezifikation den Thinker implementieren

        //Hier kommt spaeter der Thinker hin

        waitpid(pid, NULL, 0); //Wartet auf sein Kind und beendet sich nach diesem

    }

    shmctl(shmID,IPC_RMID, NULL);
    fclose(logdatei);
    free(conf);
    shmdt(pf);
    shmdt(shm);
    return EXIT_SUCCESS;
}
