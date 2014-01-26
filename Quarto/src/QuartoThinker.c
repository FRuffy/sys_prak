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
#include "Errmmry.h"
#include "ParseServerReply.h"


/**
 * Umsetzung SpielfeldNr => Spielfeldkoordinaten (wie von Server benoetigt)
 * UNICOLOR DUENN ECKIG GANZ
 * Anfang der KI
 *
 * @param SpielfeldNr
 * @return Spielfeldkoordinaten (wie von Server benoetigt)
 */
char* formatMove(int move) {
	if (move == 0)
		{ return "A4"; }
	if (move == 1)
		{ return "B4"; }
	if (move == 2)
		{ return "C4"; }
	if (move == 3)
		{ return "D4"; }
	if (move == 4)
		{ return "A3"; }
	if (move == 5)
		{ return "B3"; }
	if (move == 6)
		{ return "C3"; }
	if (move == 7)
		{ return "D3"; }
	if (move == 8)
		{ return "A2"; }
	if (move == 9)
		{ return "B2"; }
	if (move == 10)
		{ return "C2"; }
	if (move == 11)
		{ return "D2"; }
	if (move == 12)
		{ return "A1"; }
	if (move == 13)
		{ return "B1"; }
	if (move == 14)
		{ return "C1"; }
	if (move == 15)
		{ return "D1"; }
	return NULL ;
}

/**
 * Testet ob ein Stein noch verfuegbar ist
 *
 * @param Pointer auf Spielfeld, zufaellig ausgewaehlter Stein
 * @return "setzbarer" Stein
 */
int testStone(sharedmem * shm, int stone) {
	int i = 0;

	for (i = 0; i < 16; i++) {
		if (stone == *(shm->pf + i) || stone == shm->StoneToPlace) {
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

/**
 * Waehlt einen zufaelligen Stein aus, testet (mittels testStone) dessen Verfuegbarkeit
 *
 * @param Pointer auf Spielfeld
 * @return "setzbarer" Stein
 */
void chooseStone(sharedmem * shm) {
	int check = 0;
	int stone;

	while (check == 0) {
		stone = rand() % 16;
		if (testStone(shm, stone) == EXIT_SUCCESS) {
			check = 1;
		}
	}

	shm->nextStone = stone;
}

/**
 * Ueberprueft ob ein Feld besetzt ist
 *
 * @param Pointer auf zu pruefendes (string) Feld
 * @return 0 falls ok, sonst 1
 */
int checkField(char * field) {
	int i;
	int occupied = 0;

	for (i = 0; i < 64; i = i + 4) {
		if (field[i] != '*') {
			occupied++;
		}
	}

	if (occupied == 15) {
		return EXIT_FAILURE;
	} else {
		return EXIT_SUCCESS;
	}

}

/**
 * Berechnet naechsten Spielzug
 *
 * @param SHM
 * @return berechneter Spielzug
 */
int think(sharedmem * shm) {
	srand(time(NULL));

	if (strcasecmp(shm->gameName, "Quarto") && (shm->fieldX != shm->fieldY)) {
		perror("\nBei Quarto muss die Spielfeldhoehe gleich der breite sein!\n");
		return EXIT_FAILURE;
	}

	int i;
	int move = -1;
	int backupStone = 0;
	char* field = malloc(sizeof(char) * 5 * 16);
	addchar(field);
	char* backupField = malloc(sizeof(char) * 4);
	addchar(backupField);

	if (strcasecmp(shm->gameName, "Quarto") && shm->fieldX == 4) {
		convertGameFieldQuarto4x4(shm, field);
		printGameFieldQuarto4x4(field);
	} else {
		/* Trivial-KI (weil Spielfeld nicht 4x4 ist) */
		printGameField(shm);
		if (*(shm->pf) == -1) {
			strcpy(shm->nextField, formatMove(0));
		} else {
			strcpy(shm->nextField, formatMove(1));
		}

		if (shm->StoneToPlace == 0) {
			shm->nextStone = 1;
		} else {
			shm->nextStone = 0;
		}

		return EXIT_SUCCESS;
	}

	printf("\n\nWir denken...\n");

	if (checkField(field) != 0) {
		/* Suche freien Platz auf Spielfeld */
		for (i = 0; i < 16; i++) {
			move = i;

			if (*(shm->pf + move) == -1) {
				strcpy(shm->nextField, formatMove(move));
				shm->nextStone = -1;
				return EXIT_SUCCESS;
			}
		}

		printf("\nDas Spielfeld ist bereits voll, das ist aber komisch!\n");
		return EXIT_FAILURE;
	}

	move = calculateMove(shm, field, 1);

	if (move != -1) {
		printf("\n#=====================#");
		printf("\n# Wir gewinnen jetzt! #");
		printf("\n#=====================#\n");
		strcpy(shm->nextField, formatMove(move / 4));
		chooseStone(shm);

	} else {
		/* Das Feld durchlaufen und eine Position finden, die dem Gegner nicht den Sieg ermoeglicht
		 * Tests ergaben: Bei 350-450 durchlaeufen wuerde ein Sockettimeout ausgeloest. 100 sind relativ sicher.  */
		for (i = 0; i < 100; i++) {
			move = rand() % 16;

			if (*(shm->pf + move) == -1) {
				strcpy(shm->nextField, formatMove(move));

				if (formatMove(move) == NULL ) {
					perror("\nFehler bei der Konvertierung eines Spielzuges!\n");
					return EXIT_FAILURE;
				}

				/* Zum Ueberpruefen, ob der Gegner mit unserer Auswahl gewinnen kann,
				 * fuehren wir unseren Zug aus und rufen calculateMove erneut aus
				 * vorher machen wir ein Backup und stellen dieses nachher wieder her
				 * Genauer:
				 * Unser berechneter Zug wird jetzt ins Spielfeld eingetragen und ein
				 * zufaelliger naechster Stein ausgewaehlt. Dann wird das Spielfeld
				 * ins Binaerformat konvertiert "StoneToPlace" war urspruenglich der
				 * Stein, den wir vom Gegner bekommen haben, wir ueberschreiben diesen
				 * mit unserem zufaellig ausgewaehlen Stein und pruefen ob so ein Sieg
				 * moeglich ist, falls ja wird ein neuer Stein und ein neues Feld
				 * ausgewaehlt und von neuem begonnen */

				backupStone = shm->StoneToPlace;
				*(shm->pf + move) = shm->StoneToPlace;
				convertGameFieldQuarto4x4(shm, field);
				chooseStone(shm);
				shm->StoneToPlace = shm->nextStone;

				if (calculateMove(shm, field, 1) == -1) {
					/* Das machen wir, der Gegner kann mit unserem letzten Zug und dem fuer Ihn ausgewaehlten Stein nicht gewinnen */
					shm->StoneToPlace = backupStone;
					*(shm->pf + move) = -1;
					convertGameFieldQuarto4x4(shm, field);
					printf("\nMit Stein %d kann der Gegner nicht gewinnen!\n", shm->nextStone);
					return EXIT_SUCCESS;

				} else {
					/* Das machen wir nicht! So wuerden wir verlieren (bei intelligentem Gegner) */
					shm->StoneToPlace = backupStone;
					*(shm->pf + move) = -1;
					convertGameFieldQuarto4x4(shm, field);
				}
			}

		}

		printf("\n#============================================================#");
		printf("\n# Oh nein, wir haben keine Alternative und werden verlieren! #");
		printf("\n#============================================================#\n");
		fflush(stdout);
	}

	return EXIT_SUCCESS;
}

/* calculateMove uebernimmt den string stones der in der printfunktion bereits konvertiert wurde und liest diesen aus
 * (printf("\n%s",stones); zum besseren Verstaendnis). Das Feld ist insgesamt 64 Zeichen lang und ein Stein hat
 * 4 Zeichen, d.h. wir vergleichen die binaeren Eigentschaften vertikal und horizontal.
 * Horizontal: stones[((i+4)%16)+(i/16)*16+j]);
 * (i+4)%16): waehlt beispielswiese den Nachbarstein aus, falls i+4 >16 fangen wir einfach von vorn an
 * (i/16)*16+j: i waeht die i-te Reihe und j waehlt die j-te Eigenschaft aus die zu vergleichen ist.
 * Vertikal: stones[(i+16)%64+j])
 * (i+16)%64: waehlt die Zeile die zu vergleichen ist ( Ein Zeilensprung ist +16)
 * rechts-links diagonal: stones[(i+20)%80+j])
 * (i+20)%80: waehlt das diagonalfeld von 0-60 in 20er Schritten
 * links-rechts diagonal: stones[i%48+12+j]
 * i%48+12: waehlt das diagonalfeld in 12er Schritten wobei mindestens bzw. hoechstens12-48 moeglich ist
 * Hat das Programm eine Loesung gefunden gibt es eine Loesung aus, diese wird durch 4 geteilt um die korrekte Position
 * zu ermitteln und an domove weitergegeben
 * Mit silent=1 wird verhindert, dass die Ausgabe bei jedem test ob der Gegner gewinnen kann angezeigt wird */

int calculateMove(sharedmem *shm, char* stones, int silent) {
	char* stone = malloc(sizeof(char) * 5);
	addchar(stone);
	int i, j;

	/* Stein, der zu setzen ist, wird separat gespeichert */
	byte_to_binary(shm->StoneToPlace, stone);

	/* Suche nach einem Platz der zum Sieg fuehrt */
	for (i = 0; i < 64; i = i + 4) {
		/* Ist der Platz frei wird die KI ausgefuehrt */
		if (stones[i] == '*') {
			for (j = 0; j < 4; j++) {

				/* horizontal */
				if (((stone[j]) == (stones[((i + 4) % 16) + (i / 16) * 16 + j]))
						&& ((stone[j]) == (stones[((i + 8) % 16) + (i / 16) * 16 + j]))
						&& ((stone[j]) == (stones[((i + 12) % 16) + (i / 16) * 16 + j]))) {
					if (silent != 1) {
						printf("\nHorizontale Loesung gefunden (%s)! ", formatMove(i / 4));
					}
					return i;
				}

				/* vertikal */
				if (((stone[j]) == (stones[(i + 16) % 64 + j]))
						&& ((stone[j]) == (stones[(i + 32) % 64 + j]))
						&& ((stone[j]) == (stones[(i + 48) % 64 + j]))) {
					if (silent != 1) {
						printf("\nVertikale Loesung gefunden (%s)! ", formatMove(i / 4));
					}
					return i;
				}

				/* rechts -> links diagonal */
				if (i % 20 == 0) {
					if (((stone[j]) == (stones[(i + 20) % 80 + j]))
							&& ((stone[j]) == (stones[(i + 40) % 80 + j]))
							&& ((stone[j]) == (stones[(i + 60) % 80 + j]))) {
						if (silent != 1) {
							printf("\nDiagonale Loesung gefunden! %s\n", formatMove(i / 4));
						}
						return i;
					}
				}

				/* links -> rechts diagonal */
				if ((i != 0) && (i % 12) == 0) {
					if (((stone[j]) == (stones[i % 48 + 12 + j]))
							&& ((stone[j]) == (stones[(i + 12) % 48 + 12 + j]))
							&& ((stone[j]) == (stones[(i + 24) % 48 + 12 + j]))) {
						if (silent != 1) {
							printf("\nDiagonale Loesung gefunden! %s\n", formatMove(i / 4));
						}
						return i;
					}
				}

			}
		}
	}

	return -1;
}

/**
 * Konvertiert mittels Funktion byte_to_binary das Spielfeld
 * (Darstellung der Steine nicht decimal, sondern binaer)
 *
 * @param SHM, Pointer auf Steine
 * @return berechneter Spielzug
 */
int convertGameFieldQuarto4x4(sharedmem * shm, char* stones) {
	stones[0] = '\0';
	char* stone = malloc(sizeof(char) * 5);
	int i, j;

	for (i = 0; i < shm->fieldY; i++) {
		for (j = 0; j < shm->fieldX; j++) {
			if (*(shm->pf + j + i * shm->fieldY) == -1) {
				strcat(stones, "****");
			} else {
				byte_to_binary(*(shm->pf + j + i * shm->fieldY), stone);
				strcat(stones, stone);
			}
		}
	}

	free(stone);
	return EXIT_SUCCESS;
}

/**
 * Gibt das Spielfeld speziell fuer ein Quarto 4x4 Spiel aus
 *
 * @param Pointer auf Speicher mit konvertiertem Spielfeld
 * @return - (nur Ausgabe auf Bildschirm)
 */
int printGameFieldQuarto4x4(char* stones) {
	int i, j;
	printf("\n+");

	for (i = 3; i >= 0; i--) {
		printf("-------");
	}

	printf("+");

	for (i = 0; i < 4; i++) {
		printf("\n+%29c", '+');
		printf("\n+ %d:", i + 1);
		for (j = 0; j < 4; j++) {
			printf(" %c%c%c%c ", stones[i * 16 + j * 4], stones[i * 16 + j * 4 + 1], stones[i * 16 + j * 4 + 2], stones[i * 16 + j * 4 + 3]);
		}
		printf(" +");
	}

	printf("\n+%29c\n+", '+');

	for (i = 3; i >= 0; i--) {
		printf("-------");
	}

	printf("+");
	return EXIT_SUCCESS;
}

/**
 * Konvertiert in 4-stellige Binaerdarstellung
 *
 * @param Integer-Wert des Spielsteines, Pointer fuer Ergebnis
 * @return Ins Binaerformat konvertierter Spielstein
 */
int byte_to_binary(int n, char* pointer) {
	int c, d, count;
	count = 0;

	if (pointer == NULL ) {
		return EXIT_FAILURE;
	}

	for (c = 3; c >= 0; c--) {
		d = n >> c;
		if (d & 1) {
			*(pointer + count) = 1 + '0';
		} else {
			*(pointer + count) = 0 + '0';
		}
		count++;
	}

	*(pointer + count) = '\0';
	return EXIT_SUCCESS;
}
