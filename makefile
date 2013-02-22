COMP=gcc
FLAGS=-g

z827: z827.o
	$(COMP) $(FLAGS) z827.o -o z827

z827.o: z827.c
	gcc -c $(FLAGS) z827.c

test: z827
	testing/ztest | tee testResult
	more testResult

clean:
	rm -f *.o
	rm -f testResult

erase:
	rm -f *.o
	rm -f testResult
	rm -f z827
