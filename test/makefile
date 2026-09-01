

win:
	cls
	gcc server.c ../models/protocol.c -o server.exe -lws2_32
lin:
	clear
	@gcc server.c ../models/protocol.c -o server.out
	@./srrver.out

winC:
	cls
	@gcc client.c ../models/protocol.c -o client.exe -lws2_32
	@./client.exe