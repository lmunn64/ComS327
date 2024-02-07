#struct: main.c struct.c struct.h
#	gcc main.c struct.c -o struct
#not best way

stack: stack.o main.o
	gcc stack.o main.o -o stack

stack.o: stack.c stack.h
	gcc -g stack.c -c 

#-g is symbols for debugging

main.o: main.c stack.c stack.h
	gcc main.c -c

clean:
	rm -f *.o stack
