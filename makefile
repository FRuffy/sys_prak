CC=gcc
CFLAGS=-c -Wall -Werror
LDFLAGS=
SOURCES=Main.c PerformConnection.c InitConnection.c Config.c Errmmry.c AuxiliaryFunctions.c Playtime.c ServerInfo.c Select.c QuartoAI.c Thinker.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=client

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(EXECUTABLE) $(OBJECTS) log.txt
