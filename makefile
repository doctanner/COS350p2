COMP=gcc
FLAGS=-g

z827: z827.o
	$(COMP) $(FLAGS) z827.o -o z827

z827.o: z827.c
	gcc -c $(FLAGS) z827.c

test: z827
	testing/ztest

clean:
	rm -f *.o

erase:
	rf -f *.0
	rm -f z827
