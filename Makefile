CC=g++ -Wall
all: main

main: helper.o main.o queue.o
	$(CC) -pthread -o main helper.o main.o queue.o

main.o: helper.cc main.cc queue.o
	$(CC) -c helper.cc main.cc queue.cc

queue.o: queue.cc queue.h
	$(CC) -c queue.cc

tidy:
	rm -f *.o core

clean:
	rm -f main producer consumer *.o core
