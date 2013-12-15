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
    sharedmem *shm;
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

    if (pipe(fd) < 0)
    {
        perror ("Fehler beim Einrichten der Pipe.");
        exit(EXIT_FAILURE);
    }

    pid_t pid = 0;
    pid = fork();

    /* Ab hier wird in 2 Prozesse, dem Think und dem Connector, aufgespalten */

    if ((pid) < 0)
    {
        fprintf(stderr, "Fehler bei fork(): %s\n", strerror(errno));
        writelog(logdatei,AT);
        shmdt(shm);
        shmctl(shmID,IPC_RMID, NULL);
        fclose(logdatei);
        free(conf);
        return EXIT_FAILURE;
    }
    else if (pid == 0)
    {
        //Kind - soll laut Spezifikation die Verbindung herstellen (performConnection() ausfuehren)

        close(fd[1]); // schliesst input ende von der Pipe
        initConnection(argc, argv,shm,conf);
        shmdt(pf);
        shmdt(shm);

    }
    else
    {
        //Elternprozess - soll laut Spezifikation den Thinker implementieren
        firstsig=0;
        close(fd[0]);

        /**
        *Signal Handler.
        *

        * Definiert, wie auf ein Signal reagiert werden soll.
        *
        * @param  Wert des Signals.
        */
        void sig_handler(int signo)
        {
            if (signo == SIGUSR1)
            {
                printf("Signal nummer %d \n",firstsig);
                if (firstsig==0)            // Falls das Signal zum ersten mal ankommt, wird  pf-SHM erzeugt und attached, sonst geht es gleich mit thinker() weiter
                {
                    firstsig++;
                    if (pfID == 0)
                    {
                        pfID = shmget(KEY, (sizeof(short)*(shm->fieldX)*(shm->fieldX)*(shm->fieldY)), 0775 );
                        writelog(logdatei,AT);
                        if (pfID < 1)
                        {
                            printf("KIND: Error: No pf-SHM");

                        }
                        pf = shmat(pfID, 0, 0);                        writelog(logdatei,AT);
 //pf einhaengen
                    }
                    printf("\n%d\n",pfID);


                    if (pf == (void *) -1) //Im Fehlerfall pointed pf auf -1
                    {
                        fprintf(stderr, "Fehler, pf-shm: %s\n", strerror(errno));
                        writelog(logdatei,AT);
                    }
                }
                if (thinker(shm)!=0)
                    perror("VATER: thinker konnte nicht in pipe schreiben \n");
            }

        }
        int status;
        pid_t result;

        if (signal(SIGUSR1, sig_handler) == SIG_ERR)   // setzt den Signal Handler auf
        {
            perror("An error occurred while setting a signal handler.\n");
            return EXIT_FAILURE;
        }
        do
        {

            result = waitpid(pid, &status, WNOHANG);                         writelog(logdatei,AT);

            if (result!=0) printf("VATER: Beende mich selbst... \n"); // ueberprueft ob Kind noch existiert
        }
        while (result == 0); // so lange Kind noch existiert
        shmdt(pf);
        shmdt(shm);


    }

    shmctl(shmID,IPC_RMID, NULL);
    shmctl(pfID,IPC_RMID, NULL); //zerstoere pf SHM
    fclose(logdatei);
    free(conf);
    return EXIT_SUCCESS;
}
