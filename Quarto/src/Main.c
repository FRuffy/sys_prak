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
#include "QuartoThinker.h"
#include "Config.h"
#include "InitConnection.h"
#include "ReactToSig.h"

int main(int argc, char** argv) {

	/* Die Struktur, die die Konfigurationsparameter der Datei speichert */
	config_struct *conf;
	/* Pipe */
	int fd[2];
	logdatei = fopen("log.txt", "w+");
	conf = calloc(5, sizeof(config_struct));

	/* Initialisierung des Shared Memory */
	sharedmem *shm;
	int shmID;
	int shmSize = sizeof(struct sharedmem);

	shmID = shmget(IPC_PRIVATE, shmSize, IPC_CREAT | IPC_EXCL | 0775);

	if (shmID < 1) {
		perror("\nNo SHM\n");
		free(conf);
		fclose(logdatei);
		return EXIT_FAILURE;
	}

	/* SHM einhaengen */
	shm = shmat(shmID, 0, 0);

	/* Im Fehlerfall pointed shm auf -1 */
	if (shm == (void *) -1) {
		perror("Fehler, shm");
		writelog(logdatei, AT);
		return EXIT_FAILURE;
	}
	/* Setze die ID auf 0 fuer ggf. auftretende Fehler vor Initialisation */
	shm->pfID = 0;

	if (pipe(fd) < 0) {
		perror("\nFehler beim Einrichten der Pipe.\n");
		return EXIT_FAILURE;
	}

	pid_t pid = 0;
	pid = fork();

	/* Ab hier wird in 2 Prozesse, dem Thinker und dem Connector, aufgespalten */

	if ((pid) < 0) {
		perror("\nFehler bei fork()");
		writelog(logdatei, AT);
		shmdt(shm);
		fclose(logdatei);
		close(fd[0]);
		close(fd[1]);
		return EXIT_FAILURE;
	} else if (pid == 0) {
		/* Kind-Prozess soll laut Spezifikation die Verbindung herstellen (init/performConnection() ausfuehren) */

		/* Schliesst input ende von der Pipe */
		close(fd[1]);

		/* SignalHandler */
		void signal_handler(int signum) {

			/* Abfangen von CTRL + C mit Signal "SIGINT" */
			if (signum == SIGINT) {
				reactToSig(shm, 1, conf, fd, shmID);
			}
		}

		if (signal(SIGINT, signal_handler) == SIG_ERR ) {
			perror("\nFehler beim aufsetzen des Signal Handlers!\n");
			close(fd[0]);
			free(conf);
			fclose(logdatei);
			return EXIT_FAILURE;
		}

		/* Ueberpruefe ob die angegebene Game-ID ueberhaupt die richtige Laenge hat oder existiert */
		if (argc == 1 || (strlen(argv[1])) >= MAXGAMEID) {
			printf("\nDer uebergebene Parameter hat nicht die korrekte Laenge!\n");
			close(fd[0]);
			free(conf);
			fclose(logdatei);
			return EXIT_FAILURE;
		} else {
			if (argc == 3) {
				/* Falls Custom-config angegeben wurde */
				if (openConfig(argv[2], conf) != 0) {
					close(fd[0]);
					free(conf);
					fclose(logdatei);
					return EXIT_FAILURE;
				}
			} else {
				/* Sonst Standard-config */
				if (openConfig("client.conf", conf) != 0) {
					close(fd[0]);
					free(conf);
					fclose(logdatei);
					return EXIT_FAILURE;
				}
			}
			strcpy(shm->gameID, argv[1]);
		}

		initConnection(shm, conf, fd);
		freeall();

	} else {
		/* Elternprozess - soll laut Spezifikation den Thinker implementieren */

		/* Lese-Seite der Pipe wird geschlossen */
		close(fd[0]);

		/* SignalHandler */
		void signal_handler(int signum) {
			if (signum == SIGUSR1) {
				reactToSig(shm, 0, conf, fd, shmID);
			}
			/* Abfangen von CTRL + C mit Signal "SIGINT" */
			else if (signum == SIGINT) {
				reactToSig(shm, 1, conf, fd, shmID);
			}
		}
		if (signal(SIGINT, signal_handler) == SIG_ERR
				|| signal(SIGUSR1, signal_handler) == SIG_ERR ) {
			perror("\nFehler beim Aufsetzen des Signal Handlers\n");
			close(fd[1]);
			free(conf);
			fclose(logdatei);
			return EXIT_FAILURE;
		}

		int status;
		pid_t result;

		/* Ueberprueft ob Kind-Prozess noch existiert */
		do {
			usleep(50000);
			result = waitpid(shm->pidKid, &status, WNOHANG);

		} while (result == 0);

		/* Shared Memory Segmente detachen und zur Loeschung markieren */
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
		freeall();
	}
	close(fd[0]);
	close(fd[1]);
	free(conf);
	fclose(logdatei);
	return EXIT_SUCCESS;
}
