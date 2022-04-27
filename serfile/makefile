all : ser

ser : ser.o socket.o thread.o
	gcc -o ser ser.o socket.o thread.o -lpthread -lcrypto

ser.o : ser.c 
	gcc -c ser.c -g 

socket.o : socket.c
	gcc -c socket.c -g

thread.o : thread.c
	gcc -c thread.c -g

clean :
	rm -f *.o 
