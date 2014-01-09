#ifndef Select_h
#define Select_h

int waitforfds(int socket,char* buffer,sharedmem * shm); // nur ein Vorschlag der Parametrisierung, bitte feststellen,
                                                        //was tatsaechlich benoetigt wird
int doMove(int sock, char* buffer);

#endif
