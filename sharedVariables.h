#ifndef sharedVariables
#define sharedVariables

/* Alle Variablen und Funktionen die zwischen den einzelnen Modulen geteilt werden */


typedef struct config_struct {

    char hostname[100];
    char gamekindname[20];
    char version[5];
    char playernumber[5];
    int portnumber;


} config_struct;


int openConfig(char* name);
int performConnection(int socket);

config_struct *conf; // Die Struktur, die die Konfigurationsparameter der Datei speichert
int initConnection(int argc, char** argv);
FILE* logdatei;

struct sharedmem {
	pid_t pidDad;
	pid_t pidKid;
	char gameID[30]; //Name des Spiels
	int playerNumber; //Eigene Spielernummer
	int playerCount; //Anzahl Spieler
} ;

struct sharedmem *shm;


#endif
