#ifndef errmmry_h
#define errmmry_h

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

extern int varcount;
int *varadresses[20];
extern char *errorstring;
int addfree(int* adr);
int freeall();
int writelog(FILE *filename,const char *location);
#endif
