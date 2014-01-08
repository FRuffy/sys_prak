#ifndef config_h
#define config_h

int openConfig(char* name, config_struct* conf);
int checkName(char* name, char* wert, config_struct* conf);
FILE* openFile(char* name);
int readConfig(FILE* configFile, config_struct* conf);

#endif
