#ifndef Thinker_h
#define Thinker_h

int think( sharedmem *shm);
char* formatMove(int move);
char* formatMove1(int move);
int testStone(sharedmem * shm, int stone);
void chooseStone(sharedmem * shm);
int calculateMove(sharedmem *shm, char* stones);

#endif
