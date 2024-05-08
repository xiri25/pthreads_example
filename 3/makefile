CC=gcc
CFLAGS=-Wall -Wextra
LDFLAGS=-pthread

.PHONY: all clean

all: pthread3_mutex_lock pthread3_mutex_trylock

pthread3_mutex_lock: main.c
	$(CC) $(CFLAGS) -DALWAYS_LOCK=1 -o $@ $^ $(LDFLAGS)
pthread3_mutex_trylock: main.c
	$(CC) $(CFLAGS) -DALWAYS_LOCK=0 -o $@ $^ $(LDFLAGS)
clean:
	rm -f pthread3_mutex_lock pthread3_mutex_trylock
