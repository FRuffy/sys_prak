#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SharedVariables.h"
#include "Errmmry.h"
#include "PerformConnection.h"


/**
 *  Funktion, die unsere Konfigurationsdatei ausliest und Werte der Struktur zuweist
 *
 * @param Pointer auf Name, Wert, Struktur
 * @return 0 falls Konfigurationsdaten korrekt, 1 falls der Parameter nicht gefunden wurde
 */
int checkName(char* name, char* wert, config_struct* conf) {
	if (strcasecmp(name, "Hostname") == 0) {
		memcpy(conf->hostname, wert, strlen(wert));
		return EXIT_SUCCESS;
	} else if (strcasecmp(name, "Version") == 0) {
		memcpy(conf->version, wert, strlen(wert));
		return EXIT_SUCCESS;
	} else if (strcasecmp(name, "Spieltyp") == 0) {
		memcpy(conf->gamekindname, wert, strlen(wert));
		return EXIT_SUCCESS;
	} else if (strcasecmp(name, "Portnummer") == 0) {
		conf->portnumber = strtol(wert, (char**) NULL, 10);
		return EXIT_SUCCESS;
	} else if (strcasecmp(name, "Spielernummer") == 0) {
		memcpy(conf->playernumber, wert, strlen(wert));
		return EXIT_SUCCESS;
	} else {
		printf(
				"Parameter nicht gefunden! Bitte pruefen ob die Namen korrekt geschrieben oder die Konfigurationsdatei sauber ist!\n");
		return EXIT_FAILURE;
	}
}

/**
 * Standarprozedur um Datei zu oeffnen
 *
 * @param Pointer auf den Namen der Datei
 * @return Datei
 */
FILE* openFile(char* name) {

	FILE* file = NULL;

	if (name == NULL ) {
		perror("\nVon nichts kommt nichts.\n");
		return file;
	}
/* Falls die config nicht im selben Order ist, soll im Parent Folder gesucht werden */
	if ((file = fopen(name, "r")) == NULL ) {

		char* temp = malloc(sizeof(char) * 256);
		addchar(temp);
		antistrcat(name, "../", temp);

		if ((file = fopen(temp, "r")) == NULL ) {
			perror(
					"\nEs ist ein Problem mit der Konfigurationsdatei aufgetreten");
			return file;
		}
	}

	return file;
}

/**
 * Liest die Datei aus, prueft den Parameternamen und weist den korrekten Wert der Struktur zu
 * count zaehlt die Zeilen der Datei
 *
 * @param config Datei, Struktur
 * @return 0 wenn korrekter Wert zugewiesen
 */
int readConfig(FILE* configFile, config_struct* conf) {

	int count = 0;
	char* pName = malloc(sizeof(char) * 128 * 10);
	addchar(pName);
	char* pValue = malloc(sizeof(char) * 128 * 10);
	addchar(pValue);
	char* buffer = malloc(sizeof(char) * 256);
	addchar(buffer);

	while ((fgets(buffer, 127, configFile)) != NULL ) {

		if (buffer[0] != '\n' && buffer[0] != '#') {
			sscanf(buffer, " %[^ =] =%*[ ] %s \n", &pName[count * 128],&pValue[count * 128]);
			checkName(&pName[count * 128], &pValue[count * 128], conf);
			count++;
		}
	}

	return EXIT_SUCCESS;
}

/**
 * Oeffnet die Datei und liest den Parameter aus der Datei aus, um sie in die Struktur zu schreiben
 *
 * @param Poineter auf Datei Name, Struktur
 * @return 0 falls Datei existiert
 */
int openConfig(char* name, config_struct* conf) {

	FILE* file;
	file = openFile(name);

	if (file == NULL ) {
		return EXIT_FAILURE;
	}

	readConfig(file, conf);
	fclose(file);
	return EXIT_SUCCESS;
}

