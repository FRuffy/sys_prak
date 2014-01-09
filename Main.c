#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <signal.h>
#include "SharedVariables.h"
#include "Errmmry.h"
#include "AuxiliaryFunctions.h"
#include "Thinker.h"
#include "InitConnection.h"

FILE *logdatei; // Die Logdatei, die Fehler bestimmter Systemfunktionen mitzeichnet und den Ort angibt

int main (int argc, char** argv ) {
    config_struct *conf; // Die Struktur, die die Konfigurationsparameter der Datei speichert
    logdatei =fopen("log.txt","w+");
    conf = calloc(5,sizeof(config_struct));
    
    /* Initialisierung der Shared Memory */
    sharedmem *shm;
    int shmID;
    int shmSize = sizeof(struct sharedmem);

    shmID = shmget(IPC_PRIVATE, shmSize, IPC_CREAT | IPC_EXCL | 0775);

    if (shmID < 1) {
        printf("Error: No SHM");
        free(conf);
        fclose(logdatei);
        return EXIT_FAILURE;
    }

    shm = shmat(shmID, 0, 0); //SHM einhaengen ;

    //Im Fehlerfall pointed shm auf -1
    if ( shm == (void *) -1) {
        fprintf(stderr, "Fehler, shm: %s\n", strerror(errno));
        writelog(logdatei,AT);
        return EXIT_FAILURE;
    }

    if (pipe(fd) < 0) {
        perror ("Fehler beim Einrichten der Pipe.");
        return EXIT_FAILURE;
    }

    pid_t pid = 0;
    pid = fork();

    /* Ab hier wird in 2 Prozesse, dem Thinker und dem Connector, aufgespalten */

    if ((pid) < 0) {
        fprintf(stderr, "Fehler bei fork(): %s\n", strerror(errno));
        writelog(logdatei,AT);
        shmdt(shm);
        shmctl(shmID,IPC_RMID, NULL);
        fclose(logdatei);
        return EXIT_FAILURE;
    } else if (pid == 0) {
        //Kind - soll laut Spezifikation die Verbindung herstellen (performConnection() ausfuehren)
        close(fd[1]); // schliesst input ende von der Pipe
        initConnection(argc, argv,shm,conf);
        shmdt(shm->pf);
        shmdt(shm);
        free(conf);
    } else {
        //Elternprozess - soll laut Spezifikation den Thinker implementieren
        close(fd[0]);
        void signal_handler(int signum) {
            int err;
            (void) signal;
            if (signum == SIGUSR1) {
                shm->pf = shmat(shm->pfID, 0, 0);
                writelog(logdatei,AT);
                printf("\nSPIELFELDID VATER %d\n",*(shm->pf));
                 //Sicherstellen, dass SIGUSR1 vom Kind kam
                if (shm->pleaseThink == 1) {
                    shm->pleaseThink = 0;
                    think(shm);
                    char* reply = malloc(sizeof(char)*15);
                    sprintf(reply,"PLAY %s,%d",shm->nextField,shm->nextStone);
                    err = 	write (fd[1], reply, 15); //Spielzug in Pipe schreiben
                    if (err <0) {
                        perror("Fehler bei Write");
                       //Fehlerbehandlung?
                    }
                    shm->thinking = 0; // Denken beendet
                    free(reply);
                }
            }
        }
        signal(SIGUSR1, signal_handler);
        int status;
        pid_t result;
        do {
            result = waitpid(pid, &status, WNOHANG); // writelog(logdatei,AT);
            // ueberprueft ob Kind noch existiert
            if (result!=0) {
                printf("VATER: Beende mich selbst... \n");
            }
        }
        while (result == 0); // warte so lange Kind existiert
        shmdt(shm->pf);
        shmdt(shm);
        shmctl(shmID,IPC_RMID, NULL);
        shmctl(KEY,IPC_RMID, NULL); //zerstoere pf SHM
        free(conf);
    }
    
    fclose(logdatei);
    return EXIT_SUCCESS;
}
