

win:
	cls
	gcc server.c ../models/protocol.c ../models/transport.c -o server.exe -lws2_32
lin:
	clear
	@gcc server.c ../models/protocol.c ../models/transport.c -o server.out
	@./server.out

winC:
	cls
	@gcc client.c ../models/protocol.c ../models/transport.c -o client.exe -lws2_32
	@./client.exe