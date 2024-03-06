CC=g++
CFLAGS=-Wall -Wextra -std=c++11
TARGET=server client

all: $(TARGET)

server: server.o server_main.o threadPool.o
	$(CC) $(CFLAGS) server.o server_main.o threadPool.o -o server -lpthread

server.o: src/server.cpp
	$(CC) $(CFLAGS) -c src/server.cpp -lpthread

server_main.o: src/server_main.cpp
	$(CC) $(CFLAGS) -c src/server_main.cpp

threadPool.o: src/threadPool.cpp
	$(CC) $(CFLAGS) -c src/threadPool.cpp -lpthread

client: client.o client_main.o
	$(CC) $(CFLAGS) client.o client_main.o -o client

client.o: src/client.cpp
	$(CC) $(CFLAGS) -c src/client.cpp

client_main.o: src/client_main.cpp
	$(CC) $(CFLAGS) -c src/client_main.cpp

clean:
	rm -f *.o

fclean: clean
	rm -f $(TARGET)