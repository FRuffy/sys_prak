CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=Prolog.c performConnection.c config.c sharedVariables.h
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=client

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm $(EXECUTABLE) $(OBJECTS) log.txt

