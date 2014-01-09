#ifndef Thinker_h
#define Thinker_h

void think( sharedmem *shm);
char* formatMove(int move);
char* formatMove1(int move);
int testStone(sharedmem * shm, int stone);
void chooseStone(sharedmem * shm);
void think(sharedmem * shm);

#endif
