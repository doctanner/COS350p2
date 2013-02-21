COMP=gcc
FLAGS=-g

: z827.o
	$(COMP) $(FLAGS) z827.o -o bin/z827

z827.o: z827.c
	cd bin/
	gcc -c $(FLAGS) ./z827.c
	cd ../

test: z827.o
	cd bin
	../testing/ztest

clean:
	rm -f *.o
