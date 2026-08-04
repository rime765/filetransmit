build:	
	clear
	gcc client.c accementclient.c -o client.out
	gcc server.c accementserver.c -o server.out
	mv client.out ./test/client.out
	@./server.out