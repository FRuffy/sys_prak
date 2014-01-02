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
//
/**
 * Umsetzung SpielfeldNr => Spielfeldkoordinaten (wie von Server benoetigt)
 * UNICOLOR DÜNN ECKIG GANZ
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
/* Meine temporäre Notlösung um das Problem des umgedrehten Spielfelds zu lösen */
char* formatMove1(int move) //Input sollte eigentlich sein:
{
    if (move == 0)  return "A4"; //12
    if (move == 1)  return "B4"; //13
    if (move == 2)  return "C4"; //14
    if (move == 3)  return "D4"; //15
    if (move == 4)  return "A3"; //8
    if (move == 5)  return "B3"; //9
    if (move == 6)  return "C3"; //10
    if (move == 7)  return "D3"; //11
    if (move == 8)  return "A2"; //4
    if (move == 9)  return "B2"; //5
    if (move == 10) return "C2"; //6
    if (move == 11) return "D2"; //7
    if (move == 12) return "A1"; //0
    if (move == 13) return "B1"; //1
    if (move == 14) return "C1"; //2
    if (move == 15) return "D1"; //3
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
    printGameFieldQuarto4x4(shm);
	//printGameField(shm);
	printf("\nStarting to Think\n");

    srand(time(NULL));
    int check = 0;
    int move;

/* Kalkuliere naechsten Zug. Falls Sieg möglich setze dort, sonst random */
     move = calculateMove(shm);
     if(move != -1)  {
        printf("\nMOVE: %d\n",move);
   strcpy(shm->nextField, formatMove1(move/4));
 }
    // Suche freien Platz auf Spielfeld
  else  { while( check == 0)
    {
    	move = rand()%16;
        if (*(shm->pf + move) == -1 )
            check= 1;


        	strcpy(shm->nextField, formatMove(move));
    }
    // Wofuer ist das ? ? ? ? ? (ich kapier es nicht, drum auskommentiert) Flo - 25.12.2013
    //if (formatMove(move)==NULL) {
    //return NULL; }
  }
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
