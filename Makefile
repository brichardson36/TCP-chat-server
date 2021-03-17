all:
	$(CC) -Wall chatserver.c -O2 -std=c11 -lpthread -o chatserver
	$(CC) -Wall chatclient.c -O2 -std=c11 -lpthread -o chatclient
debug:
	$(CC) -Wall -g chatserver.c -O2 -std=c11 -lpthread -o chatserver_dbg
	$(CC) -Wall -g chatclient.c -O2 -std=c11 -lpthread -o chatclient_dbg

clean:
	$(RM) -rf chatserver chatclient chatserver_dbg chatclient_dbg