all: threaded

threaded: main.o visibility.o
	g++ main.o visibility.o -o threaded.exe

main.o: main.cpp visibility.hpp
	g++ -c main.cpp -o main.o

visibility.o: visibility.cpp visibility.hpp
	g++ -c visibility.cpp -o visibility.o

clean:
	rm -f *.o threaded