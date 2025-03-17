PKGS=sdl2 glew
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm

brodnick: ./src/main.c
	$(CC) $(CFLAGS) -o broadnick ./src/main.c $(LIBS)
