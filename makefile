CC=gcc
CFLAGS=-c -Wall -Werror
LDFLAGS=
SOURCES=Main.c performConnection.c initConnection.c config.c errmmry.c auxiliaryFunctions.c Playtime.c ServerInfo.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=client

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm $(EXECUTABLE) $(OBJECTS) log.txt

