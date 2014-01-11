#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SharedVariables.h"
#include "Errmmry.h"

/* Funktion, die unsere Konfigdatei ausliest und Werte der Struktur zuweist */

//Prüfe Parametername
int checkName(char* name, char* wert, config_struct* conf) {
    if (strcasecmp(name,"Hostname") == 0) {
        memcpy(conf->hostname,wert,strlen(wert));
        return EXIT_SUCCESS;
    } else if (strcasecmp(name,"Version") == 0) {
        memcpy(conf->version,wert,strlen(wert));
        return EXIT_SUCCESS;
    } else if (strcasecmp(name,"Spieltyp") == 0) {
        memcpy(conf->gamekindname,wert,strlen(wert));
        return EXIT_SUCCESS;
    } else if (strcasecmp(name,"Portnummer") == 0) {
        conf->portnumber =  strtol(wert,(char**) NULL, 10);
        return EXIT_SUCCESS;
    } else if (strcasecmp(name,"Spielernummer") == 0) {
        memcpy(conf->playernumber,wert,strlen(wert));
        return EXIT_SUCCESS;
    } else {
        printf("Parameter nicht gefunden, bitte pruefen ob die Konfigurationsdatei korrekt ist!\n");
        return EXIT_FAILURE;
    }
}

//Standarprozedur um Datei zu öffnen
FILE* openFile(char* name) {
    FILE* file = NULL;

    if (name == NULL) {
        // Akt. z50 (printfs entfernen bzw. durch perror, stdout etc. ersetzen (z.B z50))
        printf("\nVon nichts kommt nichts.\n");
        return file;
    }

    if ((file=fopen(name,"r")) == NULL) {
        perror("Konfigurationsdatei konnte nicht geoeffnet werden oder existiert nicht!");
        return file;
    }
    
    return file;
}


int readConfig(FILE* configFile, config_struct* conf) {
    //ToDo(Malloc Fehlerabfrage fehlt (z66-67))
    int count = 0; //Zähle die Zeilen der Datei
    int i;
    char* pName = malloc(sizeof(char)*128*10);  addchar(pName);
    char* pValue = malloc(sizeof(char)*128*10); addchar(pValue);
    //while ((fscanf(configFile," %[^ =] = %s \n",&pName[count*128],&pValue[count*128])) != EOF)
    //fix:
    while ((fscanf(configFile," %[^ =] =%*[ ] %s \n",&pName[count*128],&pValue[count*128])) != EOF) {
        count++;
    }

    for (i = 0; i< count; i++) {
        //printf("\nNAME: %s WERT: %s\n",&pName[i*128],&pValue[i*128]); //Debugging
        checkName(&pName[i*128],&pValue[i*128],conf); //Prüfe den Parameternamen und weise anschließend den korrekten Wert der Struktur zu
    }

    return EXIT_SUCCESS;
}


int openConfig(char* name,config_struct* conf) {
    //name = malloc(sizeof(char)*64);
    //name = "/home/r/ruffy/Systempraktikum/Systempraktikum/client.conf";
    FILE* file;
    file  = openFile(name); // Öffne Datei

    if (file == NULL) {
        return EXIT_FAILURE;
    }

    readConfig(file,conf);  //Lese Parameter aus der Datei aus und schreibe sie in die Struktur
    fclose(file);
    return EXIT_SUCCESS;
}
