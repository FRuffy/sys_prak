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
#include "SharedVariables.h"
#include "Thinker.h"
#include "AuxiliaryFunctions.h"
#include "Errmmry.h"
#include "QuartoAI.h"
#include "Errmmry.h"


// Anfang der KI
/**
 * Umsetzung SpielfeldNr => Spielfeldkoordinaten (wie von Server benoetigt)
 * UNICOLOR DÜNN ECKIG GANZ
 * @param  SpielfeldNr
 * @return Spielfeldkoordinaten(wie von Server benoetigt)
 */
char* formatMove(int move) {
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
char* formatMove1(int move) { //Input sollte eigentlich sein:
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
int testStone(sharedmem * shm, int stone) {
    int i=0;
    for (i=0; i<16; i++) {
        if (stone == *(shm->pf+i) || stone == shm->StoneToPlace) {
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
void chooseStone(sharedmem * shm) {
    srand(time(NULL));
    int check = 0;
    int stone;

    while( check== 0) {
        stone = rand()%16;
        if(testStone(shm, stone)==EXIT_SUCCESS) {
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
int think(sharedmem * shm) {
     char* field = malloc(sizeof(char)*5*16);   addchar(field);
    srand(time(NULL));
    int check = 0;
    int move = -1;
     if (strcasecmp(shm->gameName, "Quarto") && shm->fieldX ==4) {
      printGameFieldQuarto4x4(shm,field);
     }
   else {
    printGameField(shm);
   }

    printf("\nStarting to Think\n");

    /* Kalkuliere naechsten Zug. Falls Sieg möglich setze dort, sonst random */
   if (shm->thinkTime >500) {

        move = calculateMove(shm,field);
   }
   else {
    printf("\n Wir denken lieber nicht zu hart nach!\n");
   }

    if(move != -1) {
        printf("\nMOVE: %d\n",move);
        strcpy(shm->nextField, formatMove1(move/4));
    } else { // Suche freien Platz auf Spielfeld
    	while (check == 0) {
    		move = rand()%16;
        	if (*(shm->pf + move) == -1) {
            		check= 1;
            	}
		strcpy(shm->nextField, formatMove(move));
    	}

    if (formatMove(move)==NULL) {
            perror("\nFehler bei der Konvertierung eines Spielzuges!\n");
    return EXIT_FAILURE; }
    }
    chooseStone(shm);
    return EXIT_SUCCESS;
}
