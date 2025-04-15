CC = gcc
CFLAGS = -g -Wall -pthread
SRC = $(wildcard *.c)
TARGETS = $(basename $(SRC))

all:$(TARGETS)

# 动态目标规则：支持 make filename.c
%:%.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean all rebuild

clean:
	rm -rf $(basename $(SRC)) *.o

rebuild:clean all