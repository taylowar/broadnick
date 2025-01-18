CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags sdl2`
LIBS=`pkg-config --libs sdl2` -lm

brodnick: main.c
	$(CC) $(CFLAGS) -o broadnick main.c $(LIBS)

