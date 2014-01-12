#ifndef AuxiliaryFunctions_h
#define AuxiliaryFunctions_h

void sendReplyFormatted(int sock, char* reply);
char* antistrcat(char* dest, char* src, char* temp);
int readGameField(char *buffer,sharedmem * shm);
int printGameField( sharedmem *shm);

#endif
