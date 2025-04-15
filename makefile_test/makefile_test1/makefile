TARGET = ./bin/test1
SRC = $(wildcard ./src/*.c)
OBJS = $(patsubst ./src/%.c, ./obj/%.o, $(SRC))
CFLAGS = -c
INC = ./inc/
CC = gcc
RM = -rm -f

$(TARGET):$(OBJS)
	$(CC) $^ -I $(INC) -o $@

./obj/%.o:./src/%.c
	$(CC) $(CFLAGS) -I $(INC) $< -o $@

.PHONY:clean all

clean:
	$(RM) $(OBJS) $(TARGET)

all:
	echo "SRC $(SRC)"
	echo "OBJ $(OBJS)"
	echo "CC $(CC)"
	echo "RM $(RM)"
