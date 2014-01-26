#ifndef ParseServerReply_h
#define ParseServerReply_h

int readGameField(char *buffer, sharedmem * shm);
int printGameField(sharedmem *shm);
int parseServerReply(int sock, char* buffer, sharedmem * shm);

#endif
