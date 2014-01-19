#ifndef Select_h
#define Select_h

int waitforfds(int socket, char* buffer, sharedmem * shm, int fd[]); // nur ein Vorschlag der Parametrisierung, bitte feststellen, was tatsaechlich benoetigt wird
int doMove(int sock, char* buffer, int fd[]);

#endif
