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
#include "QuartoThinker.h"
#include "AuxiliaryFunctions.h"
#include "Errmmry.h"

// Anfang der KI
/**
 * Umsetzung SpielfeldNr => Spielfeldkoordinaten (wie von Server benoetigt)
 * UNICOLOR DUENN ECKIG GANZ
 * @param  SpielfeldNr
 * @return Spielfeldkoordinaten(wie von Server benoetigt)
 */
char* formatMove(int move)
{
    if (move == 0)  return "A4";
    if (move == 1)  return "B4";
    if (move == 2)  return "C4";
    if (move == 3)  return "D4";
    if (move == 4)  return "A3";
    if (move == 5)  return "B3";
    if (move == 6)  return "C3";
    if (move == 7)  return "D3";
    if (move == 8)  return "A2";
    if (move == 9)  return "B2";
    if (move == 10) return "C2";
    if (move == 11) return "D2";
    if (move == 12) return "A1";
    if (move == 13) return "B1";
    if (move == 14) return "C1";
    if (move == 15) return "D1";
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
int think(sharedmem * shm)
{
    if (shm->fieldX != shm->fieldY)
    {
        printf("Bei Quarto muss die Spielfeldhoehe gleich der -breite sein!");
        return EXIT_FAILURE;
    }
    char* field = malloc(sizeof(char)*5*16);
    addchar(field);
    srand(time(NULL));
    int check = 0;
    int move = -1;
    if (strcasecmp(shm->gameName, "Quarto") && shm->fieldX == 4)
    {
        printGameFieldQuarto4x4(shm,field);
    }
    else
    {
        // Trivial-KI (weil Spielfeld nicht 4x4 ist)
        printGameField(shm);
        if (*(shm->pf) == -1)
        {
            strcpy(shm->nextField, formatMove(0));
        }
        else
        {
            strcpy(shm->nextField, formatMove(1));
        }

        if (shm->StoneToPlace == 0)
        {
            shm->nextStone = 1;
        }
        else
        {
            shm->nextStone = 0;
        }
        return EXIT_SUCCESS;
    }

    printf("\nStarting to Think\n");

    /* Kalkuliere naechsten Zug. Falls Sieg moeglich setze dort, sonst random */
    if (shm->thinkTime > 500 || shm->thinkTime ==0)
    {

        move = calculateMove(shm,field);
    }
    else
    {
        printf("\n Wir denken lieber nicht zu hart nach!\n");
    }

    if(move != -1)
    {
        printf("Wir gewinnen jetzt!\n");
        strcpy(shm->nextField, formatMove(move/4));
    }
    else
    {
        // Suche freien Platz auf Spielfeld
        while (check == 0)
        {
            move = rand()%16;
            if (*(shm->pf + move) == -1)
            {
                check= 1;
            }

            strcpy(shm->nextField, formatMove(move));
        }

        if (formatMove(move)==NULL)
        {
            perror("\nFehler bei der Konvertierung eines Spielzuges!\n");
            return EXIT_FAILURE;
        }
    }
    chooseStone(shm);
    return EXIT_SUCCESS;
}
/** Unsere "kluge" KI!
Sie übernimmt den string stones der in der printfunktion bereits konvertiert wurde und liest diesen aus
(printf("\n%s",stones); zum besseren Verstaendnis). Das Feld ist insgesamt 64 Zeichen lang und ein Stein hat
4 Zeichen, d.h. wir vergleichen die binären Eigentschaften vertikal und horizontal.
Horizontal: stones[((i+4)%16)+(i/16)*16+j]);
(i+4)%16): wählt beispielswiese den Nachbarstein aus, falls i+4 >16 fangen wir einfach von vorn an
(i/16)*16+j: i wäht die i-te Reihe und j wählt die j-te Eigenschaft aus die zu vergleichen ist.
 Vertikal: stones[(i+16)%64+j])
(i+16)%64: wählt die Zeile die zu vergleichen ist ( Ein Zeilensprung ist +16)
rechts-links diagonal: stones[(i+20)%80+j])
(i+20)%80: wählt das diagonalfeld von 0-60 in 20er Schritten
links-rechts diagonal: stones[i%48+12+j]
i%48+12: wählt das diagonalfeld in 12er Schritten wobei mindestens  bzw. höchstens12-48 möglich ist

Hat das Programm eine Lösung gefunden gibt es eine Lösung aus, diese wird durch 4 geteilt um die korrekte Position
zu ermitteln und an domove weitergegeben

**/
int calculateMove(sharedmem *shm, char* stones )
{

    char* stone = malloc(sizeof(char)*5);
    addchar(stone);
    int i, j;

    /* Stein, der zu setzen ist, wird separat gespeichert */
    byte_to_binary(shm->StoneToPlace,stone);


    //printf("\n%s",stones);
    //printf("\n%s",stone);

    /*Suche nach einem Platz der zum Sieg fuehrt */
    for (i=0; i<64; i=i+4)
    {
        //Ist der Platz frei wird die KI ausgeführt
        if (stones[i] == '*')
        {
            for (j=0; j<4; j++)

            {
           //     printf("\n%c %c %c %c",stone[j],stones[((i+4)%16)+(i/16)*16+j],stones[((i+8)%16)+(i/16)*16+j],stones[((i+12)%16)+(i/16)*16+j]);
//printf(" Part 2: %c %c %c %c",stone[j],stones[(i+16)%64+j],stones[(i+32)%64+j],stones[(i+48)%64+j]);
                 //horizontal
                if (((stone[j]) == (stones[((i+4)%16)+(i/16)*16+j])) && ((stone[j]) == (stones[((i+8)%16)+(i/16)*16+j])) &&((stone[j]) == (stones[((i+12)%16)+(i/16)*16+j])))
                {
                    printf("\nLoesung gefunden (%d)! ",i);


                    return i;
                }
                //vertikal
                if (((stone[j]) == (stones[(i+16)%64+j])) && ((stone[j]) == (stones[(i+32)%64+j])) &&((stone[j]) == (stones[(i+48)%64+j])))
                {
                    printf("\nLoesung gefunden! %d\n",i);


                    return i;
                }
                 //rechts-> links diagonal
                if(i %20 == 0)
                {

                    if (((stone[j]) == (stones[(i+20)%80+j])) && ((stone[j]) == (stones[(i+40)%80+j])) &&((stone[j]) == (stones[(i+60)%80+j])))
                    {
                        printf("\nDiagonale Loesung gefunden! %d\n",i);


                        return i;
                    }
                }
                 //links-> links diagonal
                   if((i != 0) && (i %12) == 0)
                {

                    if (((stone[j]) == (stones[i%48+12+j])) && ((stone[j]) == (stones[(i+12)%48+12+j])) &&((stone[j]) == (stones[(i+24)%48+12+j])))
                    {
                        printf("\nDiagonale Loesung gefunden! %d\n",i);


                        return i;
                    }
                }
            }
        }
    }


    return -1;
}



/**
 * Gibt das Spielfeld speziell fuer ein Quarto 4x4 Spiel aus
 */
int printGameFieldQuarto4x4(sharedmem * shm, char* stones)
{

    stones[0] = '\0';
    char* stone = malloc(sizeof(char)*5);

    int i,j;
    printf("\n+");

    for (i=shm->fieldY-1; i>=0; i--)
    {
        printf("-------");
    }

    printf("+");

    for (i=0; i<shm->fieldY; i++)
    {
        printf("\n+%29c", '+');
        printf("\n+ %d:", i+1);
        for (j=0; j<shm->fieldX; j++)
        {
            if (*(shm->pf+j+i*shm->fieldY)==-1)
            {
                strcat(stones,"****");
                printf(" **** ");
            }
            else
            {
                //strcpy(stone,byte_to_binary(*(shm->pf+j+i*shm->fieldY)));
                byte_to_binary(*(shm->pf+j+i*shm->fieldY),stone);
                strcat(stones,stone);
                printf(" %s ", stone);
            }
        }
        printf(" +");
    }

    printf("\n+%29c\n+", '+');

    for (i=shm->fieldY-1; i>=0; i--)
    {
        printf("-------");
    }

    printf("+");
    free(stone);
    return EXIT_SUCCESS;
}

/**
 * Konvertiert in 4-stellige Binaerdarstellung
 */
int byte_to_binary(int n, char* pointer)
{
    int c, d, count;
    count = 0;
    if (pointer == NULL)
    {
        return EXIT_FAILURE;
    }

    for (c=3; c>=0 ; c--)
    {
        d = n >> c;
        if ( d & 1 )
        {
            *(pointer+count) = 1 + '0';
        }
        else
        {
            *(pointer+count) = 0 + '0';
        }
        count++;
    }

    *(pointer+count) = '\0';
    return  EXIT_SUCCESS;
}