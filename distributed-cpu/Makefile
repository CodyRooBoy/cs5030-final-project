all: distributed_cpu

distributed_cpu: main.o visibility.o
	mpicxx main.o visibility.o -o distributed_cpu.exe

main.o: main.cpp visibility.hpp
	mpicxx -c main.cpp -o main.o

visibility.o: visibility.cpp visibility.hpp
	mpicxx -c visibility.cpp -o visibility.o

clean:
	rm -f *.o *.exe