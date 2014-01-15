#ifndef Errmmry_h
#define Errmmry_h

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

FILE *logdatei;
int *varadr_int[50];
char *varadr_char[50];
int addint(int* intadr);
int addchar(char* charadr);
int freeall();
int writelog(FILE *filename, const char *location);

#endif

