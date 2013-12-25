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
#include <signal.h>
#include "thinker.h"

FILE *logdatei; // Die Logdatei, die Fehler bestimmter Systemfunktionen mitzeichnet und den Ort angibt

int main (int argc, char** argv )
{
    config_struct *conf; // Die Struktur, die die Konfigurationsparameter der Datei speichert
    logdatei =fopen("log.txt","w+");
    conf = calloc(5,sizeof(config_struct));
    /* Initialisierung der Shared Memory */
    int shmID;
    int shmSize = sizeof(struct sharedmem);

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

    if (pipe(fd) < 0)
    {
        perror ("Fehler beim Einrichten der Pipe.");
        exit(EXIT_FAILURE);
    }

    pid_t pid = 0;
    pid = fork();

    /* Ab hier wird in 2 Prozesse, dem Thinker und dem Connector, aufgespalten */

    if ((pid) < 0)
    {
        fprintf(stderr, "Fehler bei fork(): %s\n", strerror(errno));
        writelog(logdatei,AT);
        shmdt(shm);
        shmctl(shmID,IPC_RMID, NULL);
        fclose(logdatei);
        return EXIT_FAILURE;
    }
    else if (pid == 0)
    {
        //Kind - soll laut Spezifikation die Verbindung herstellen (performConnection() ausfuehren)
        close(fd[1]); // schliesst input ende von der Pipe
        initConnection(argc, argv,shm,conf);
        shmdt(shm->pf);
        shmdt(shm);
        free(conf);
    }
    else
    {
        //Elternprozess - soll laut Spezifikation den Thinker implementieren
    	close(fd[0]);
    	signal(SIGUSR1, signal_handler);
        int status;
        pid_t result;

        do
        {
            result = waitpid(pid, &status, WNOHANG);                        // writelog(logdatei,AT);
            if (result!=0) printf("VATER: Beende mich selbst... \n"); // ueberprueft ob Kind noch existiert
        }
        while (result == 0); // warte so lange Kind existiert

        free(conf);
        shmdt(shm);
        shmctl(shmID,IPC_RMID, NULL);
        shmctl(KEY,IPC_RMID, NULL); //zerstoere pf SHM
    }
    fclose(logdatei);
    return EXIT_SUCCESS;
}
