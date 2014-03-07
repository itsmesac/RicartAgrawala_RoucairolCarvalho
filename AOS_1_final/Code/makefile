all:node

node: node.o Initialize.o ConfigFileParser.o
	gcc node.o Initialize.o ConfigFileParser.o -o node -lpthread

node.o: node.c
	gcc -c node.c

Initialize.o: Initialize.c
	gcc -c Initialize.c

ConfigFileParser.o: ConfigFileParser.c
	gcc -c ConfigFileParser.c

clean:
	rm -rf *.o node


