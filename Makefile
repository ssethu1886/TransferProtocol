CC=gcc
CFLAGS=-Wall -Wextra -Werror

all: clean build

default: build

build: server.cpp client.cpp
	gcc -Wall -Wextra -o server server.cpp -lstdc++ -Wno-unused-variable
	gcc -Wall -Wextra -o client client.cpp -lstdc++ -Wno-unused-variable

clean:
	rm -f server client output.txt project2.zip

zip: 
	zip project2.zip server.cpp client.cpp utils.h Makefile README
