CC = gcc
CFLAGS = -Wall -Werror -g

all: memory_manager.a

bitmap.o: bitmap.c
	$(CC) $(CFLAGS) -c bitmap.c

memory_manager.o: memory_manager.c memory_manager.h
	$(CC) $(CFLAGS) -c memory_manager.c

memory_manager.a: bitmap.o memory_manager.o
	ar rcs memory_manager.a bitmap.o memory_manager.o

# test_main: test_main.c memory_manager.a
# 	$(CC) $(CFLAGS) -o test_main test_main.c memory_manager.a

clean:
	rm -f *.o *.a
