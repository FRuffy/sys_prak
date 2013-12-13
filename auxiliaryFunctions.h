#ifndef auxiliaryFunctions
#define auxiliaryFunctions


int openConfig(char* name, config_struct* conf);
int performConnection(int sock,sharedmem * shm, config_struct* conf);
char*  recvServerInfo(char* buffer,sharedmem * shm);
int checkServerReply(int sock, char* buffer,sharedmem * shm);
int initConnection(int argc, char** argv,sharedmem * shm, config_struct* conf);
void sendReplyFormatted(int sock, char* reply);
char* antistrcat(char* dest, char* src);
int readGameField(char *buffer,sharedmem * shm);
int printGameField();
char* think (sharedmem *shm);


#endif
