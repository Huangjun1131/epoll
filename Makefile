all:
	gcc server.c -o server -DDEBUG
	gcc client.c -o client -DDEBUG
clean:
	rm server client

