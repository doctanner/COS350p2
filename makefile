# AUTHOR: James Tanner
# COURSE: COS 350 - Systems Programming
# PURPOSE: Program 2

COMP=gcc
FLAGS=-g

z827: z827.o
	$(COMP) $(FLAGS) z827.o -o z827
	rm -f *Result
	rm -f writeUp

z827.o: z827.c
	gcc -c $(FLAGS) z827.c

testResult: z827
	clear
	chmod +x support/testScript
	support/testScript 2>&1 | tee testResult
	chmod -x support/testScript

test: z827 testResult
	more testResult

writeUp: z827 testResult
	chmod +x support/writeScript
	support/writeScript 2>&1 | tee writeUp
	chmod -x support/writeScript

print: z827 writeUp
	a2ps writeUp

man: manpage
	man ./manpage
   
clean:
	rm -f *.o
	rm -f *Result
	rm -f *.txt*
	

erase: clean
	rm -f writeUp
	rm -f z827

flat: erase
	uninst/flatten
