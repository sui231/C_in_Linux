SRCS:= $(wildcard *.c)
OBJS:= $(patsubst %.c,%.o,$(SRCS))
CC:= gcc
TARGET:= main
$(TARGET): $(OBJS)
	$(CC) $^ -o $@ -pthread -g
%.o:%.c
	$(CC) -c $^ -o $@ -g
clean:
	$(RM) $(OBJS) server $(TARGET)
rebuild:clean $(TARGET)
.PHONY:clean rebuild