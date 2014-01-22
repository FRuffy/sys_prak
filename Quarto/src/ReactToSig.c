#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "SharedVariables.h"
#include "Errmmry.h"
#include "QuartoThinker.h"

/**
 * Signal "SIGUSR1": Der Vater ruft den Thinker auf.
 * Signal "SIGINT" : CTRL + C wurde gedrueckt - Vater und Kind beenden sich
 *
 * @param SHM, signal (0 = Zug soll berechnet werden | 1 = CTRL + C wurde gedrueckt),
 * config_struct für free-Befehl, fd[] fuer Spielzug, shmID zum detatchen zur zerstoren des shm
 * @return 0 bei Sielzug in Pipe geschrieben, -1 bei Fehler
 */
int reactToSig(sharedmem* shm, int signal, config_struct *conf, int fd[],
		int shmID) {
	/* Zug soll berechnet werden */
	if (signal == 0) {
		/* Signal SIGUSR1 wurde empfangen */
		int err;
		(void) signal;
		shm->pf = shmat(shm->pfID, 0, 0);

		/* Sicherstellen, dass SIGUSR1 vom Kind kam */
		if (shm->pleaseThink == 1) {
			shm->pleaseThink = 0;
			if (think(shm) != 0) {
				return EXIT_FAILURE;
			}

			char* reply = calloc(15, sizeof(char));
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

	} else if (signal == 1) {
		/* CTRL + C wurde gedrueckt */
		fclose(logdatei);
		free(conf);
		freeall();
		if (shm->pidKid == getpid()) {
			/* Kind */
			close(fd[0]);
			close(shm->sock);
			shmdt(shm->pf);
			shmdt(shm);
		} else {
			/* Vater */
			close(fd[1]);
			if (shm->pfID != 0) {
				shmdt(shm->pf);
				if (shmctl(shm->pfID, IPC_RMID, NULL ) == -1) {
					perror("\nParent: Fehler bei Zerstoerung von pf \n");
				}
			}
			shmdt(shm);
			if (shmctl(shmID, IPC_RMID, NULL ) == -1) {
				perror("\nParent: Fehler bei Zerstoerung von shm \n");
			}
		}

		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
