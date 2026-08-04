build:	
	clear
	gcc server.c accementserver.c -o server.out
	gcc client.c accementclient.c -o ./test/client.out
	./server.out

client:	
	clear
	rm *.c
	rm *.out
	./client.out