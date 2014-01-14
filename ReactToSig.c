#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "SharedVariables.h"
#include "Errmmry.h"
#include "QuartoThinker.h"


int reactToSig(sharedmem* shm){
	int err;
    (void) signal;
	shm->pf = shmat(shm->pfID, 0, 0);
	    writelog(logdatei,AT);
	    //Sicherstellen, dass SIGUSR1 vom Kind kam
	    if (shm->pleaseThink == 1) {
	        shm->pleaseThink = 0;
	        think(shm);
	        char* reply = malloc(sizeof(char)*15); addchar(reply);
	        if (shm->nextStone == -1)
            {
                sprintf(reply,"PLAY %s",shm->nextField);
            }
            else {
	        sprintf(reply,"PLAY %s,%d",shm->nextField,shm->nextStone);
            }
	        err = write (fd[1], reply, 15); //Spielzug in Pipe schreiben
	        if (err <0) {
	            perror("Fehler bei Write");
	            return EXIT_FAILURE;
	        }
	    shm->thinking = 0; // Denken beendet
	    }
	return EXIT_SUCCESS;
}
