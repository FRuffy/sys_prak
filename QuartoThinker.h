#ifndef QuartoThinker_h
#define QuartoThinker_h

int think( sharedmem *shm);
char* formatMove(int move);
char* formatMove1(int move);
int testStone(sharedmem * shm, int stone);
void chooseStone(sharedmem * shm);
int calculateMove(sharedmem *shm, char* stones);
int printGameFieldQuarto4x4(sharedmem * shm, char* stones);
int byte_to_binary(int n, char* container);

#endif
