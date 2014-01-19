#ifndef PerformConnection_h
#define PerformConnection_h

void sendReplyFormatted(int sock, char* reply);
char* antistrcat(char* dest, char* src, char* temp);
int performConnection(int sock, sharedmem * shm, config_struct* conf, int fd[]);

#endif
