#ifndef Playtime_h
#define Playtime_h

int readGameField(char *buffer, sharedmem * shm);
int printGameField(sharedmem *shm);
int checkServerReply(int sock, char* buffer, sharedmem * shm);

#endif
