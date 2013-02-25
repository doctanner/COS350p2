# AUTHOR: James Tanner
# COURSE: COS 350 - Systems Programming
# PURPOSE: Program 2

COMP=gcc
FLAGS=-g

z827: z827.o
	$(COMP) $(FLAGS) z827.o -o z827

z827.o: z827.c
	gcc -c $(FLAGS) z827.c

test: z827
	testing/ztest | tee testResult
	more testResult

writeup: z827
	./writeScript > JTWriteUp 2>&1
	more JTWriteUp

man: manpage
	man ./manpage
   
clean:
	rm -f *.o
	rm -f testResult

erase:
	rm -f *.o
	rm -f testResult
	rm -f z827
