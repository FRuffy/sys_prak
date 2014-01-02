#ifndef auxiliaryFunctions
#define auxiliaryFunctions


int openConfig(char* name, config_struct* conf);
int openConfig(char* name, config_struct* conf);
int performConnection(int sock,sharedmem * shm, config_struct* conf);
char*  recvServerInfo(char* buffer,sharedmem * shm);
int checkServerReply(int sock, char* buffer,sharedmem * shm);
int initConnection(int argc, char** argv,sharedmem * shm, config_struct* conf);
void sendReplyFormatted(int sock, char* reply);
char* antistrcat(char* dest, char* src, char* temp);
int readGameField(char *buffer,sharedmem * shm);
int printGameField( sharedmem *shm);
int printGameFieldQuarto4x4(sharedmem * shm);
void loop(int sock, char* buffer, sharedmem * shm);
void think( sharedmem *shm);
char* byte_to_binary(int n);
int calculateMove(sharedmem *shm);
#endif
