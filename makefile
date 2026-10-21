CC = gcc
CFLAGS = -Wall -ansi -pedantic -g
OBJ = main.o map.o terminal.o linkedlist.o
EXEC = labyrinth

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

main.o: main.c map.h terminal.h linkedlist.h
	$(CC) -c main.c $(CFLAGS)

map.o: map.c map.h terminal.h color.h
	$(CC) -c map.c $(CFLAGS)

terminal.o: terminal.c terminal.h
	$(CC) -c terminal.c $(CFLAGS)

linkedlist.o: linkedlist.c linkedlist.h map.h
	$(CC) -c linkedlist.c $(CFLAGS)

clean:
	rm -f $(EXEC) $(OBJ)
