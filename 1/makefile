CC=gcc
CFLAGS=-Wall -Wextra
LDFLAGS=-pthread

.PHONY: all clean

all: pthread1_mutex pthread1_nomutex

pthread1_mutex: main.c
	$(CC) $(CFLAGS) -DUSE_MUTEX=1 -o $@ $^ $(LDFLAGS)

pthread1_nomutex: main.c
	$(CC) $(CFLAGS) -DUSE_MUTEX=0 -o $@ $^ $(LDFLAGS)

clean:
	rm -f pthread1_mutex pthread1_nomutex

