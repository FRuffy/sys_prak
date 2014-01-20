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

/**
 * Signal Handler des Parent Prozesses. Ruft den thinker auf, wenn das signal gesendet wurde.
 *
 * @param SHM, signal (0 = Zug soll berechnet werden | 1 = CTRL + C wurde gedrueckt), config_struct für free-Befehl
 * @return 0 falls SIGUSR vom Kind
 */
int reactToSig(sharedmem* shm, int signal, config_struct *conf, int fd[]) {
	//Zug soll berechnet werden
	if (signal == 0) {
		int err;
		(void) signal;
		shm->pf = shmat(shm->pfID, 0, 0);
		writelog(logdatei, AT);

		/* Sicherstellen, dass SIGUSR1 vom Kind kam */
		if (shm->pleaseThink == 1) {
			shm->pleaseThink = 0;
		if 	(think(shm) !=0) {
            return EXIT_FAILURE;
		    }

			char* reply = malloc(sizeof(char) * 15);
			addchar(reply);
			if (shm->nextStone == -1) {
				sprintf(reply, "PLAY %s", shm->nextField);
			} else {
				sprintf(reply, "PLAY %s,%d", shm->nextField, shm->nextStone);
			}

			/* Spielzug in Pipe schreiben */
			err = write(fd[1], reply, 15);

			if (err < 0) {
				perror("\nFehler beim Schreiben in die Pipe\n");
				return EXIT_FAILURE;
			}

			/* Denken beendet */
			shm->thinking = 0;
		}
		// CTRL + C wurde gedrueckt
	} else if (signal == 1) {
		fclose(logdatei);
		close(shm->sock);
		free(conf);
		freeall();
		printf("\nProgramm wurde durch Tasenkombination CTRL + C beendet\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
