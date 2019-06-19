CFLAGS = -pthread -lpthread -g 

objetivo1:
	gcc ${CFLAGS} -o servidor1 servidor1.c
	gcc -g -o cliente1 cliente1.c

objetivo2:
	gcc ${CFLAGS} -o servidor2 servidor2.c
	gcc -g -o cliente2 cliente2.c

objetivo3:
	gcc ${CFLAGS} -o servidor3 servidor3.c
	gcc -g -o cliente3 cliente3.c

all: objetivo1 objetivo2 objetivo3

clean:
	rm -rf cliente1 cliente2 cliente3 servidor1 servidor2 servidor3 usuarios_servidor.txt 

