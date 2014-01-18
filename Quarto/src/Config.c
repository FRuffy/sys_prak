#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SharedVariables.h"
#include "Errmmry.h"
#include "AuxiliaryFunctions.h"


/* Funktion, die unsere Konfigdatei ausliest und Werte der Struktur zuweist */

/**
 * Prüfe Parametername
 *
 * @param Pointer auf Name, Wert, Struktur
 * @return 0 falls Konfigurationsdaten korrekt
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
		printf("Parameter nicht gefunden, bitte pruefen ob die Konfigurationsdatei korrekt ist!\n");
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

	if ((file = fopen(name, "r")) == NULL ) {
		perror("Konfigurationsdatei konnte nicht geoeffnet werden oder existiert nicht!");
		return file;
	}

	return file;
}

/**
 * Liest die Datei aus, prüft den Parameternamen und weist den korrekten Wert der Struktur zu
 * count zählt die Zeilen der Datei
 *
 * @param config Datei, Struktur
 * @return 0 wenn korrekter Wert zugewiesen
 */
int readConfig(FILE* configFile, config_struct* conf) {
	//ToDo(Malloc Fehlerabfrage fehlt (z66-67))
	int count = 0;
	int i;
	char* pName = malloc(sizeof(char) * 128 * 10);
	addchar(pName);
	char* pValue = malloc(sizeof(char) * 128 * 10);
	addchar(pValue);

	while ((fscanf(configFile, " %[^ =] =%*[ ] %s \n", &pName[count * 128],	&pValue[count * 128])) != EOF) {
		count++;
	}

	for (i = 0; i < count; i++) {
		checkName(&pName[i * 128], &pValue[i * 128], conf);
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
	
char* temp = malloc(sizeof(char)*256); addchar(temp);
        antistrcat(name,"../",temp);
        FILE* file;
	file = openFile(temp);

	if (file == NULL ) {
		return EXIT_FAILURE;
	}

	readConfig(file, conf);
	fclose(file);
	return EXIT_SUCCESS;
}
