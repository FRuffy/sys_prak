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
#include <time.h>
#include "sharedVariables.h"
#include "thinker.h"
#include "auxiliaryFunctions.h"
#include "errmmry.h"


// Anfang der KI

/**
 * Umsetzung SpielfeldNr => Spielfeldkoordinaten (wie von Server benoetigt)
 *
 * @param  SpielfeldNr
 * @return Spielfeldkoordinaten(wie von Server benoetigt)
 */
char* formatMove(int move)
{
    if (move == 0)  return "A1";
    if (move == 1)  return "B1";
    if (move == 2)  return "C1";
    if (move == 3)  return "D1";
    if (move == 4)  return "A2";
    if (move == 5)  return "B2";
    if (move == 6)  return "C2";
    if (move == 7)  return "D2";
    if (move == 8)  return "A3";
    if (move == 9)  return "B3";
    if (move == 10) return "C3";
    if (move == 11) return "D3";
    if (move == 12) return "A4";
    if (move == 13) return "B4";
    if (move == 14) return "C4";
    if (move == 15) return "D4";
    return NULL;
}

/**
 *  Testet ob ein Stein noch verfuegbar ist
 *
 * @param  Pointer auf Spielfeld, zufaellig ausgewaehlter Stein
 * @return "setzbarer" Stein
 */
int testStone(sharedmem * shm, int stone)
{

    int i=0;

    for (i=0; i<16; i++)
    {
        if (stone == *(shm->pf+i) || stone == shm->StoneToPlace)
        {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;

}

/**
 *  Waehlt zufaelligen Stein aus, testet (mittels testStone) dessen verfuegbarkeit
 *
 * @param  Pointer auf Spielfeld
 * @return "setzbarer" Stein
 */
void chooseStone(sharedmem * shm)
{
    srand(time(NULL));
    int check = 0;
    int stone;

    while( check== 0)
    {
        stone = rand()%16;
        if(testStone(shm, stone)==EXIT_SUCCESS)
        {
            check = 1;
        }
    }
    shm->nextStone = stone;
}

/**
 * Berechnet naechsten Spielzug
 *
 * @param
 * @return berechneter Spielzug
 */
void think(sharedmem * shm)
{
	printGameField(shm);
	printf("\nStarting to Think\n");

    srand(time(NULL));
    int check = 0;
    int move;

    // Suche freien Platz auf Spielfeld
    while( check == 0)
    {
    	move = rand()%16;
        if (*(shm->pf + move) == -1 )
            check= 1;


        	strcpy(shm->nextField, formatMove(move));
    }
    // Wofuer ist das ? ? ? ? ? (ich kapier es nicht, drum auskommentiert) Flo - 25.12.2013
    //if (formatMove(move)==NULL) {
    //return NULL; }

    chooseStone(shm);
}

// Ende der KI

/**
* Signal Handler.
*

* Definiert, wie auf ein Signal reagiert werden soll.
*
* @param  Wert des Signals.
*//*
void signal_handler(int signum)
	{ int err;
	(void) signal;
	    if (signum == SIGUSR1)
	    {
	    	shm->pf = shmat(shm->pfID, 0, 0);

	    	//Sicherstellen, dass SIGUSR1 vom Kind kam
	    	if (shm->pleaseThink == 1) {
	    		shm->pleaseThink = 0;
	    		think(shm);

	    		char* reply = malloc(sizeof(char)*15);
//	    		if (reply == NULL) perror("Fehler bei malloc");
	    	    sprintf(reply,"PLAY %s,%d",shm->nextField,shm->nextStone);
	    	err = 	write (fd[1], reply, 15); //Spielzug in Pipe schreiben
	    	if (err <0)
            {
                perror("Fehler bei Write");
            }
	    		shm->thinking = 0; // Denken beendet
	    		free(reply);
	    	}
	    }
	}

*/
