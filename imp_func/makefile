CC = gcc
CFLAGS = -g -Wall
SRC = $(wildcard *.c)

# 动态目标规则：支持 make filename.c
%:%.c
	$(CC) $(CFLAGS) $< -o $(basename $<)

.PHONY: clean

clean:
	rm -rf $(basename $(SRC)) *.o
