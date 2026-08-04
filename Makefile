build:	
	clear
	gcc server.c accementserver.c -o server.out
	gcc client.c accementclient.c -o client.out
	./server.out