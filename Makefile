build:
	gcc client.c accementclient.c -o client.out
	gcc server.c accementserver.c -o server.out
	@./server.out