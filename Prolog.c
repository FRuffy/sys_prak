#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <netdb.h>
#include "sharedVariables.h"
#include "errmmry.h"

#define PORTNUMMER 1357
#define HOSTNAME "sysprak.priv.lab.nm.ifi.lmu.de"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

char *ID; //Die modifizierte GameID die wir mit der performConnection Funktion teilen werden
char * playerNum; // Die gewuenschte Spielernummer die wir optional angeben koennen
config_struct *conf; // Die Struktur, die die Konfigurationsparameter der Datei speichert

int main (int argc, char** argv )
{
    FILE *logdatei=fopen("log.txt","w+");
    conf = calloc(5,sizeof(config_struct));
    char *gameID;
    ID = malloc(sizeof(char)*14);
    playerNum = malloc(sizeof(char)*10);
    gameID = malloc(sizeof(char)*11);
    strcpy(ID,"ID "); // Vorbereitung der GameID fuer performConnection
	int shmID;  // ID des SHM
	struct sharedmem {
		pid_t pidDad;
		pid_t pidKid;
		char gameID2[14]; //Name des Spiels
		int playerNumber; //Eigene Spielernummer
		int playerCount; //Anzahl Spieler
	} ;
	int shmSize = sizeof(struct sharedmem);
	// sharedMem Hilfe: http://www.nt.fh-koeln.de/vogt/bs/animationen/SharedMemAnimation.pdf
	shmID = shmget(IPC_PRIVATE, shmSize, IPC_CREAT | IPC_EXCL | 0775);
	if (shmID < 1) {
		printf("Error: No SHM");
		return 0;
	}

	struct sharedmem *shm = shmat(shmID, 0, 0); //SHM einhaengen ;

	if (shm == (void *) -1) { //Im Fehlerfall pointed shm auf -1
		fprintf(stderr, "Fehler, shm: %s\n", strerror(errno)); writelog(logdatei,AT);
	}
	pid_t pid = 0;
	pid = fork();

	//Ab hier Aufspaltung in 2 Prozesse
	if ((pid) < 0) {
	    fprintf(stderr, "Fehler bei fork(): %s\n", strerror(errno)); writelog(logdatei,AT);
	}
	else if (pid == 0) {
		//Kind - soll laut Spezifikation die Verbindung herstellen (performConnection() ausfuehren)

	    //ueberpruefe ob die angegebene Game-ID ueberhaupt die richtige Laenge hat oder existiert
	    if ( argc == 1 || (strlen (argv[1])) != 11)
	    {
	        printf("Fehler: Der uebergebene Parameter hat nicht die korrekte Laenge");
	        return EXIT_FAILURE;
	    }
	    else
	    {
	        strcpy(playerNum, "PLAYER "); //Vorbereitung der Spielernummer fuer performConnection

	        if (argc == 3)
	        {
	            if (openConfig(argv[2])!= 0) //Falls Custom-config angegeben wurde
	            {
	                return EXIT_FAILURE;
	            }
	        }
	        else
	        {
	            if (openConfig("client.conf") != 0) //Sonst Standard-config
	            {
	                return EXIT_FAILURE;
	            }
	        }
	        strcpy(gameID,argv[1]);
	        printf("Deine Game ID: %s\n",gameID);
	        strcat(ID,gameID);

	    	strcpy(shm->gameID2,"ID "); // Vorbereitung der GameID fuer performConnection
	    	strcat(shm->gameID2,argv[1]);
	    	shm->pidDad = getppid(); //PID vom Vater und Eigene in SHM schreiben
			shm->pidKid = getppid();

			printf("\n\n %s \n\n", shm->gameID2);
			printf("\n\n %s \n\n", ID); //Hier mache ich morgen weiter - als sollte die Variable gameID ueberfluessig werden und durch shm->gameID bzw im moment heisst sie noch gameID2 ersetzt werden
	    }

	    // Initialisiert den fuer die Verbindung benoetigten Socket //
	    int sock = socket(AF_INET, SOCK_STREAM, 0); writelog(logdatei,AT);
	    struct sockaddr_in host;
	    struct hostent* ip;
	    ip = (gethostbyname(conf->hostname)); //uebersetze HOSTNAME in IP Adresse
	    memcpy(&(host.sin_addr),ip ->h_addr,ip ->h_length);
	    host.sin_family = AF_INET;
	    host.sin_port = htons(conf->portnumber);

	    if (connect(sock,(struct sockaddr*)&host, sizeof(host)) == 0) //Verbinde mit Server
	    {
	        printf("\nVerbindung mit %s hergestellt!\n",conf->hostname); writelog(logdatei,AT);
	    }
	    else
	    {
	        perror("\n Fehler beim Verbindungsaufbau"); writelog(logdatei,AT);
	        return EXIT_FAILURE;
	    }
	    performConnection(sock);//Fuehre Prolog Protokoll aus
	}
	else {
		//Elternprozess - soll laut Spezifikation den Thinker implementieren

        //Hier kommt spaeter der Thinker hin

		waitpid(pid, NULL, 0); //Wartet auf sein Kind und beendet sich nach diesem
		free(playerNum);
	    free(gameID);
	    free(ID);
	    free(conf);
	}

		return EXIT_SUCCESS;
}
