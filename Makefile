.PHONY : all
all : test main wait
test : test.c ars.h ars.c utils.h utils.c Makefile
	gcc -Wall -pthread -O0 -g3 -o test test.c ars.c utils.c
main : main.c ars.h ars.c utils.h utils.c Makefile
	gcc -Wall -pthread -O0 -g3 -o main main.c ars.c utils.c
wait : wait.c ars.h ars.c utils.h utils.c Makefile
	gcc -Wall -pthread -O0 -g3 -o wait wait.c ars.c utils.c
