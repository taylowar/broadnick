CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags sdl2`
LIBS=`pkg-config --libs sdl2` -lm

brodnick: ./src/main.c
	$(CC) $(CFLAGS) -o broadnick ./src/main.c $(LIBS)
