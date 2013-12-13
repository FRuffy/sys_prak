#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "sharedVariables.h"


char* formatMove(int move)
{

    if (move == 0)
    {
        return "A1";
    }
    if (move == 1)
    {
        return "B1";
    }
    if (move == 2)
    {
        return "C1";
    }
    if (move == 3)
    {
        return "D1";
    }
    if (move == 4)
    {
        return "A2";
    }
    if (move == 5)
    {
        return "B2";
    }
    if (move == 6)
    {
        return "C2";
    }
    if (move == 7)
    {
        return "D2";
    }
    if (move == 8)
    {
        return "A3";
    }
    if (move == 9)
    {
        return "B3";
    }
    if (move == 10)
    {
        return "C3";
    }
    if (move == 11)
    {
        return "D3";
    }
    if (move == 12)
    {
        return "A4";
    }
    if (move == 13)
    {
        return "B4";
    }
    if (move == 14)
    {
        return "C4";
    }
    if (move == 15)
    {
        return "D4";
    }
    return NULL;
}

int testStone(int stone,int placeStone)
{

    int i=0;
    for (i=0; i<16; i++)
    {

        if (*(pf+i) == stone || stone == placeStone)
        {

            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;

}
int chooseStone(int placeStone){
srand(time(NULL));

  int check = 0;

    int stone;


    while( check== 0) {
       stone = rand()%16;

if(testStone(stone,placeStone)==0) {

    check = 1;
}


    }

return stone;
}


char* think (sharedmem * shm) {

char* reply = malloc(sizeof(char)*15);

srand(time(NULL));


/*
int i;
    for (i=0; i<16;i++) {


printf(" %d ", *(pf+i));
    } */
    int check = 0;
    int move;

    while( check == 0) {
       move = rand()%16;
        if (*(pf+move) == -1 )
            check= 1;
    }



sprintf(reply,"PLAY %s,%d",formatMove(move),chooseStone(shm->nextStone));
printf("\n%s\n",reply);
return reply;

}
