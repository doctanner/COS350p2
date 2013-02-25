# AUTHOR: James Tanner
# COURSE: COS 350 - Systems Programming
# PURPOSE: Program 2

COMP=gcc
FLAGS=-g

z827: z827.o
	$(COMP) $(FLAGS) z827.o -o z827

z827.o: z827.c
	gcc -c $(FLAGS) z827.c

testResult: z827
	support/ztest | tee testResult

test: testResult
	more testResult

writeup: z827 testResult
	support/writeScript 2>&1 | tee writeUp
	more writeUp

man: manpage
	man ./manpage
   
clean:
	rm -f *.o

erase:
	rm -f *.o
	rm -f testResult
	rm -f writeUp
	rm -f z827
