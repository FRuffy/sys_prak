#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include "sharedVariables.h"
#include "errmmry.h"
#include "auxiliaryFunctions.h"
#include <signal.h>

#define BUFFR 512

/*Formatierte Ausgabe an den Server, \n wird benoetigt um das Ende der Uebertragung zu signalisieren */

int performConnection(int sock, sharedmem * shm, config_struct* conf) {
	//int bytesread;
	//int n = 15; // Max Groesse des Spielzug-Strings in Bytes
	//char readbuffer[n]; //Hier wird der Spielzug-String aus der Pipe abgespeichert.

	int size; //size zur Fehlerbehandlung fuer recv
	char* reader = malloc(sizeof(char) * 20);
	char* temp = malloc(sizeof(char)*256);
	char* buffer = malloc(sizeof(char) * BUFFR);
	if (reader == NULL || temp == NULL || buffer == NULL) {
		free(reader); free(temp); free(buffer); perror("Fehler bei malloc");
		return EXIT_FAILURE;
	}

	/* Teil 1: Lese die Client-Version des Servers und antworte mit eigener (formatierten) Version
	 Behandle die Antwort des Servers */

	size = recv(sock, buffer, BUFFR - 1, 0);
	sscanf(buffer, "%*s%*s%*s%s", reader);
	if (size > 0)
		buffer[size] = '\0';
	printf("\nDie Version des Servers ist: %s\n", reader);

	temp = antistrcat(conf->version, "VERSION ");
	sendReplyFormatted(sock, temp);
	free(temp);

	size = recv(sock, buffer, BUFFR - 1, 0);
	if (size > 0)
		buffer[size] = '\0';

	if (buffer[0] == '-') {
		printf("\nDer Server akzeptiert die Version %s dieses Clients nicht!\n",
				conf->version);
		free(buffer);
		free(reader);
		return EXIT_FAILURE;
	} else {
		printf(
				"\nDie Client-Version wurde akzeptiert, uebertrage Spiel-ID...\n");
	}

	/* Teil 2: Der Server erwartet Game-ID, schicke diese und behandle Antwort.
	 Falls die ID fehlerhart oder das Spiel des Servers nicht Quarto ist wird die Verbindung beendet*/

	temp = antistrcat(shm->gameID, "ID ");
	sendReplyFormatted(sock, temp);
	free(temp);
	size = recv(sock, buffer, BUFFR - 1, 0);
	if (size > 0)
		buffer[size] = '\0';
	sscanf(buffer, "%*s%*s%s", reader);

	if (buffer[0] == '-') {
		printf(
				"\nDie uebergebene Game-ID fehlt oder ist fehlerhaft! Beende Verbindung\n");
		free(buffer);
		free(reader);
		return EXIT_FAILURE;
	} else if (strcmp(reader, "Quarto") != 0) {
		printf(
				"\nDas Spiel, das der Server spielt, ist nicht Quarto! Beende Verbindung.\n");
		free(buffer);
		free(reader);
		return EXIT_FAILURE;
	} else {
		printf("\nDer Server moechte %s spielen. Und wir auch!\n", reader);
	}
	/* Teil 3.1: Hatten wir mit unserer ID Erfolg erfahren wir zunaechst den Namen des Spiels
	 und senden die ggf. uebergebene Spielernummer.
	 Anschliessend wird die Antwort des Servers auf die Nummer behandelt und Name und Nummer des Spielerplatzes ausgelesen
	 */

	size = recv(sock, buffer, BUFFR - 1, 0);
	if (size > 0)
		buffer[size] = '\0';

	sscanf(buffer, "%*s%s", shm->gameName);
	printf("\nSpiel: %s\n", shm->gameName); //Zeige Spielnamen an, schneide das "+" ab
	temp = antistrcat(conf->playernumber, "PLAYER ");
	sendReplyFormatted(sock, temp);
	free(temp);
	size = recv(sock, buffer, BUFFR - 1, 0);
	if (size > 0)
		buffer[size] = '\0';

	if (buffer[0] == '-') {
		if (buffer[2] == '-') {
			printf(
					"\nEs wurde eine ungueltige Spielernummer eingegeben! Beende Verbindung\n");
		} else {
			printf(
					"\nEs wurde kein freier Platz gefunden, versuchen sie es spaeter noch einmal!\n");
			//ACHTUNG. Diese Meldung erscheint auch, wenn man in der client.conf eine Spielernummer an gibt, die beim Erstellen des Spiels einem Computerspieler zugeordnet wurde!
		}
		free(buffer);
		free(reader);
		return EXIT_FAILURE;
	} else {
		sscanf(buffer, "%*s %*s %d %s", &(shm->player[0].playerNumber),
				(shm->player[0].playerName));
		//printf("\nDu spielst mit dem Namen %s, deine Nummer ist %d\n", shm->player[0].playerName,shm->player[0].playerNumber);
	}
	/* Teil 3.2: Hier wird der Uebergang in die Spielverlaufsphase eingeleitet.
	 Der Server sendet uns die Namen unseres Gegenspielers und des Spielernummer, wobei ueberprueft wird ob ein Spieler bereits verbunden ist.
	 */
	size = recv(sock, buffer, BUFFR - 1, 0);
	if (size > 0)
		buffer[size] = '\0';
	if (buffer[0] == '-') {
		printf("\nFehler bei Uebermittlung der Spielparameter!\n");
		free(buffer);
		free(reader);
		return EXIT_FAILURE;
	}
//Empfange die Serverdaten, falls ein Fehler hier auftritt Programm beenden
	if (recvServerInfo(buffer, shm) == NULL) {
		free(buffer);
		free(reader);
		return EXIT_FAILURE;

	}
	/* Hier faengt im Endeffekt der Connector an, der die laufende Verbindung mit dem Server haendelt
	 und mit dem Thinker zusammenarbeitet, wie das genauer implementiert wird, weiß ich leider noch nicht.
	 @Harun wuerdest du das machen?

	 Harun: muss ich mir noch ueberlegen, wenn jemand ne Idee hat bitte schreiben
	 */

	loop(sock, buffer, shm);

	printf("\n%s\n", buffer);

	free(buffer);
	free(reader);
	return EXIT_SUCCESS;
}
