#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "SharedVariables.h"
#include "AuxiliaryFunctions.h"
#include "Errmmry.h"

/**
Diese KI überprüft bei jedem freien Feld, ob bereits 3 Nachbarn mit gleicher Eigenschaft in einer Reihe legen
Ist dem so setzt returned sie das Feld für die Formatierung.
Falls keine solchen Steine gefunden werden, ist das Ergebnis negativ und die KI setzt random
**/
int calculateMove(sharedmem *shm, char* stones ) {

    char* stone = malloc(sizeof(char)*5); addchar(stone);
    int i, j;

    /* Stein, der zu setzen ist, wird separat gespeichert */
//    strcpy(stone,byte_to_binary(shm->StoneToPlace));
byte_to_binary(shm->StoneToPlace,stone);
    //printf("\n%s",stones);
    //printf("\n%s",stone);

   /*Suche nach einem Platz der zum Sieg führt */
    for (i=0; i<64; i=i+4) {
        if (stones[i] == '*') {
            for (j=0; j<4; j++) {
                /*Komplizierte Magie. */
                if (((stone[j]) == (stones[((i+4)%16)+(i/16)*16+j])) && ((stone[j]) == (stones[((i+8)%16)+(i/16)*16+j])) &&((stone[j]) == (stones[((i+12)%16)+(i/16)*16+j]))) {
                    printf("\nLoesung gefunden! %d\n",i);


                    return i;
                }
                if (((stone[j]) == (stones[(i+16)%64+j])) && ((stone[j]) == (stones[(i+32)%64+j])) &&((stone[j]) == (stones[(i+48)%64+j]))) {
                    printf("\nLoesung gefunden! %d\n",i);


                    return i;
                }
            }
        }
    }


    return -1;
}

//printf("\n%c %c %c %c",stone[j],stones[((i+4)%16)+(i/16)*16+j],stones[((i+8)%16)+(i/16)*16+j],stones[((i+12)%16)+(i/16)*16+j]);
//printf(" Part 2: %c %c %c %c",stone[j],stones[(i+16)%64+j],stones[(i+32)%64+j],stones[(i+48)%64+j]);
