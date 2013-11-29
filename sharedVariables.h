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

extern config_struct *conf;
extern int openConfig(char* name);
extern int performConnection(int socket);
extern char *ID;
extern char* playerNum;
extern struct sharedmem *shm;

#endif
