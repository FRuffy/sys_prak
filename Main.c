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
#include "ReactToSig.h"

int main(int argc, char** argv) {
	// Die Struktur, die die Konfigurationsparameter der Datei speichert
	config_struct *conf;

	logdatei = fopen("log.txt", "w+");
	conf = calloc(5, sizeof(config_struct));

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

	/* SHM einhaengen */
	shm = shmat(shmID, 0, 0);

	/* Im Fehlerfall pointed shm auf -1 */
	if (shm == (void *) -1) {
		fprintf(stderr, "Fehler, shm: %s\n", strerror(errno));
		writelog(logdatei, AT);
		return EXIT_FAILURE;
	}

	if (pipe(fd) < 0) {
		perror("Fehler beim Einrichten der Pipe.");
		return EXIT_FAILURE;
	}

	pid_t pid = 0;
	pid = fork();

	/* Ab hier wird in 2 Prozesse, dem Thinker und dem Connector, aufgespalten */

	if ((pid) < 0) {
		fprintf(stderr, "Fehler bei fork(): %s\n", strerror(errno));
		writelog(logdatei, AT);
		shmdt(shm);
		shmctl(shmID, IPC_RMID, NULL );
		fclose(logdatei);
		return EXIT_FAILURE;
	} else if (pid == 0) {
		/* Kind - soll laut Spezifikation die Verbindung herstellen (performConnection() ausfuehren);
		 * Schliesst input ende von der Pipe */
		close(fd[1]);

		/* Ueberpruefe ob die angegebene Game-ID ueberhaupt die richtige Laenge hat oder existiert */
		if (argc == 1 || (strlen(argv[1])) > 18) {
			printf("Fehler: Der uebergebene Parameter hat nicht die korrekte Laenge");
			return EXIT_FAILURE;
		} else {
			if (argc == 3) {
				/* Falls Custom-config angegeben wurde */
				if (openConfig(argv[2], conf) != 0) {
					return EXIT_FAILURE;
				}
			} else {
				/* Sonst Standard-config */
				if (openConfig("client.conf", conf) != 0) {
					return EXIT_FAILURE;
				}
			}
			strcpy(shm->gameID, argv[1]);
		}
		initConnection(shm, conf);
		shmdt(shm->pf);
		shmdt(shm);
		freeall();

	} else {
		/* Elternprozess - soll laut Spezifikation den Thinker implementieren;
		 * Lese-Seite der Pipe wird geschlossen */
		close(fd[0]);

		/* SignalHandler */
		void signal_handler(int signum) {
			if (signum == SIGUSR1) {
				reactToSig(shm);
			}
		}
		if (signal(SIGUSR1, signal_handler) == SIG_ERR ) {
			perror("Fehler beim aufsetzen des Signal Handlers!\n");
			return EXIT_FAILURE;
		}
		int status;
		pid_t result;
		do {
			result = waitpid(pid, &status, WNOHANG);
			/* Ueberprueft ob Kind noch existiert */
			if (result != 0) {
				printf("VATER: Kind ist nicht mehr vorhanden... \n");
			}
		} while (result == 0);

		printf("VATER: Zerstoere shm... \n");
		shmdt(shm->pf);
		shmdt(shm);
		if (shmctl(shmID, IPC_RMID, NULL ) == -1) {
			perror("Fehler bei Zerstoerung von shm \n");
		}
		freeall();
		printf("VATER: Beende mich selbst...\n");
	}

	free(conf);
	fclose(logdatei);
	return EXIT_SUCCESS;
}
