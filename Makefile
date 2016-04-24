#Makefile
OBJS1 = board.o serverhelper.o
OBJS2 = boardpost.o
SOURCE1 = board.c serverhelper.c
SOURCE2 = boardpost.c
HEADER = serverhelper.h
COMPHEADER = serverhelper.h.gch
OUT1 = board
OUT2 = boardpost
CC = gcc
FLAGS = -g -c

# -g option enables debugging mode
# -c flag generates object code for separate files
all: $(OBJS1) $(OBJS2)
	gcc -o $(OUT1) $(SOURCE1)
	gcc -o $(OUT2) $(SOURCE2)

# create / compile the individual files separately
board.o: board.c
	$(CC) $(FLAGS) board.c

boardpost.o: boardpost.c
	$(CC) $(FLAGS) boardpost.c

serverhelper.o: serverhelper.c
	$(CC) $(FLAGS) serverhelper.c

# clean
clean:
	rm -f $(OBJS1) $(OBJS2) $(OUT1) $(OUT2) $(COMPHEADER)

clean2:
	rm /tmp/sdi1100094/*

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)
