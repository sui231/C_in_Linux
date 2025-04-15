CC = gcc
CFLAGS = -g -Wall -pthread -I/usr/include/mysql
LDFLAGS = -L/usr/lib/mysql -lmysqlclient
SRC = $(wildcard *.c)
TARGETS = $(basename $(SRC))

all: $(TARGETS)

# Dynamic target rule: supports make filename.c
%: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

.PHONY: clean all

clean:
	rm -rf $(TARGETS) *.o
