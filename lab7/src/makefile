CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGETS = udpserver udpclient tcpserver tcpclient

all: $(TARGETS)

udpserver: udpserver.c
	$(CC) $(CFLAGS) -o $@ $<

udpclient: udpclient.c
	$(CC) $(CFLAGS) -o $@ $<

tcpserver: tcpserver.c
	$(CC) $(CFLAGS) -o $@ $<

tcpclient: tcpclient.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean