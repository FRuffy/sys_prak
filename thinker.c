#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include "sharedVariables.h"
#include "thinker.h"
#include "auxiliaryFunctions.h"
#include <time.h>

// Anfang der KI vom Fabian

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

int testStone(int stone,int placeStone, int * pf)
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
int chooseStone(int placeStone, int * pf)
{
    srand(time(NULL));

    int check = 0;

    int stone;


    while( check== 0)
    {
        stone = rand()%16;

        if(testStone(stone,placeStone, pf)==EXIT_SUCCESS)
        {

            check = 1;
        }



    }

    return stone;
}


char* think (sharedmem * shm)
{

    char* reply = malloc(sizeof(char)*15);

    srand(time(NULL));









    int check = 0;
    int move;

    while( check == 0)
    {
        move = rand()%16;
        if (*(shm->pf+move) == -1 )
            check= 1;
    }

if (formatMove(move)==NULL) {
    return NULL; }

    sprintf(reply,"PLAY %s,%d",formatMove(move),chooseStone(shm->nextStone, shm->pf));
    printf("\n%s\n",reply);
    return reply;





}

// Ender der KI vom Fabian

/**
 * Thinker.
 *
 * Schreibt einen von der KI (siehe oben) berechneten Spielzug in die Pipe
 *
 * @param  shared memory pointer.
 */
int thinker(sharedmem * shm)
{
    int n=15; // Max Groesse des Spielzug strings in Bytes.
    printf("VATER: habe ein Signal erhalten, berechne Spielzug \n");
    char* move4pipe = malloc(sizeof(char)*10);

strcpy(move4pipe,think(shm));
    if ((write (fd[1], move4pipe, n)) < 9)   // Falls kleiner 9 ist der Spielzug String falsch. Ansonsten wird in die Pipe geschrieben.
    {
        perror ("\nFehler bei write().\n");
        return EXIT_FAILURE;
    }



    else
    {
        printf("\nVATER: Thinker hat Spielzug in pipe fertiggeschrieben \n");
    }
    free(move4pipe);
    return EXIT_SUCCESS;
}
