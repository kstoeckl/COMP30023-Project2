#Kurt Stoeckl kstoeckl
# Makefile for project 2 (COMP30017)

## CC  = Compiler.
## CFLAGS = Compiler flags.
CC	= gcc
CFLAGS 	= -Wall  -pthread

default: server client

server: server.o game.o
	$(CC) $(CFLAGS) -o server server.o game.o -lm

client: client.o game.o
	$(CC) $(CFLAGS) -o client client.o game.o -lm

## Clean: Remove object files and core dump files.
clean:
		/bin/rm $(OBJ) 

## Clobber: Performs Clean and removes executable file.

clobber: clean
		/bin/rm $(EXE) 

## Dependencies

server.o: game.h
client.o: game.h
game.o: game.h