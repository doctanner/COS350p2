COMP=gcc
FLAGS=-g

p2: z827.o
	$(COMP) $(FLAGS) z827.o -o ./bin/z827

z827.o: z827.c
	gcc -c $(FLAGS) z827.c

test: z827.o
	cd bin; \
	../testing/ztest

clean:
	rm -f *.o
