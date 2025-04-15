SRC = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRC))
TARGETS = $(notdir $(basename $(SRC)))
CC = gcc

all: $(TARGETS)

%: %.o
	$(CC) -g -Wall $< -o $@

%.o: %.c
	$(CC) -c $< -o $@

.PHONY: clean all

clean:
	rm -rf $(TARGETS) $(OBJS)
