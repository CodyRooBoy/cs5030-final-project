all: serial

serial: main.o visibility.o
	g++ main.o visibility.o -o serial.exe

main.o: main.cpp visibility.hpp
	g++ -c main.cpp -o main.o

visibility.o: visibility.cpp visibility.hpp
	g++ -c visibility.cpp -o visibility.o

clean:
	rm -f *.o serial