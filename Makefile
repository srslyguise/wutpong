CC=gcc
LIBS=-lSDL -lSDL_ttf -lm

all: wutpong

wutpong: wutpong.o
	$(CC) -o wutpong wutpong.o $(LIBS)

wutpong.o: wutpong.c
	$(CC) -c -o wutpong.o wutpong.c

clean:
	rm -f wutpong.o
	rm -f wutpong
