src = $(wildcard *.c)
obj = $(patsubst %.c, %.o, $(src))

CC = gcc
target = hello1

ALL:$(target)

hello1:$(obj)
	$(CC) $^ -o $@

%.o:%.c
	$(CC) -c $< -o $@

clean:
	-rm -rf $(obj) $(target)

.PHONY:clean ALL
