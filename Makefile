


build:
	gcc client.c accement.c -o client.out
	gcc server.c accement.c -o server.out
	@./server.out