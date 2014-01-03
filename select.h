#ifndef select_h
#define select_h

int waitforfds(int socket,char* buffer,sharedmem * shm);
int doMove(int sock, char* buffer);

#endif
