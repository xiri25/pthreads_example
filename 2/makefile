CC=gcc
CFLAGS=-Wall -Wextra
LDFLAGS=-pthread

.PHONY: all clean

all: pthread2_mutex

pthread2_mutex: main.c
	$(CC) $(CFLAGS) -DUSE_MUTEX=1 -o $@ $^ $(LDFLAGS)

clean:
	rm -f pthread2_mutex
