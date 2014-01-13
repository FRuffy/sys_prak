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
#include "QuartoThinker.h"
#include "Config.h"
#include "InitConnection.h"


int main (int argc, char** argv )
{


    config_struct *conf; // Die Struktur, die die Konfigurationsparameter der Datei speichert
    logdatei =fopen("log.txt","w+");
    conf = calloc(5,sizeof(config_struct));

    /* Initialisierung der Shared Memory */
    sharedmem *shm;
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

    //Im Fehlerfall pointed shm auf -1
    if ( shm == (void *) -1)
    {
        fprintf(stderr, "Fehler, shm: %s\n", strerror(errno));
        writelog(logdatei,AT);
        return EXIT_FAILURE;
    }

    if (pipe(fd) < 0)
    {
        perror ("Fehler beim Einrichten der Pipe.");
        return EXIT_FAILURE;
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

        //überpruefe ob die angegebene Game-ID ueberhaupt die richtige Laenge hat oder existiert
        if ( argc == 1 || (strlen (argv[1])) > 18)
        {
            printf("Fehler: Der uebergebene Parameter hat nicht die korrekte Laenge");
            return EXIT_FAILURE;
        }
        else
        {
            if (argc == 3)
            {
                //Falls Custom-config angegeben wurde
                if (openConfig(argv[2],conf)!= 0)
                {
                    return EXIT_FAILURE;
                }
            }
            else
            {
                //Sonst Standard-config
                if (openConfig("client.conf",conf) != 0)
                {
                    return EXIT_FAILURE;
                }
            }
            strcpy(shm->gameID,argv[1]);
        }

        initConnection(shm,conf);
        shmdt(shm->pf);
        shmdt(shm);
        freeall();

    }
    else
    {
        //Elternprozess - soll laut Spezifikation den Thinker implementieren
        close(fd[0]);
        void signal_handler(int signum)
        {
            int err;
            (void) signal;
            if (signum == SIGUSR1)
            {
                shm->pf = shmat(shm->pfID, 0, 0);
                writelog(logdatei,AT);
                //Sicherstellen, dass SIGUSR1 vom Kind kam
                if (shm->pleaseThink == 1)
                {
                    shm->pleaseThink = 0;
                    think(shm);
                    char* reply = malloc(sizeof(char)*15);
                    addchar(reply);
                    sprintf(reply,"PLAY %s,%d",shm->nextField,shm->nextStone);
                    err =         write (fd[1], reply, 15); //Spielzug in Pipe schreiben
                    if (err <0)
                    {
                        perror("Fehler bei Write");
                        //Fehlerbehandlung?
                    }
                    shm->thinking = 0; // Denken beendet
                }
            }
        }
        signal(SIGUSR1, signal_handler);
        int status;
        pid_t result;
        do
        {
            result = waitpid(pid, &status, WNOHANG); // writelog(logdatei,AT);
            // ueberprueft ob Kind noch existiert
            if (result!=0)
            {
                printf("VATER: Kind ist nicht mehr vorhanden... \n");
            }
        }
        while (result == 0); // warte so lange Kind existiert

        printf("VATER: Zerstoere shm... \n");
        shmdt(shm->pf);
        shmdt(shm);
        if (shmctl(shmID,IPC_RMID, NULL)==-1) perror("Fehler bei Zerstoerung von shm \n");
        freeall();

        printf("VATER: Beende mich selbst...\n");

    }
    free(conf);
    fclose(logdatei);
    return EXIT_SUCCESS;
}
