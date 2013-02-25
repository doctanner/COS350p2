# AUTHOR: James Tanner
# COURSE: COS 350 - Systems Programming
# PURPOSE: Program 2

COMP=gcc
FLAGS=-g

z827: z827.o
	$(COMP) $(FLAGS) z827.o -o z827

z827.o: z827.c
	gcc -c $(FLAGS) z827.c
	rm -f testResult
	rm -f writeUp

testResult: z827
	clear
	support/ztest 2>&1 | tee testResult

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
	rm -f *Result
	rm -f *.txt*
	rm -f writeUp
	rm -f z827
