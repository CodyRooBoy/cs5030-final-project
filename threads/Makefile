CXX = g++
CXXFLAGS = -std=c++11 -pthread
LDFLAGS = -pthread

all: threaded

threaded: main.o visibility.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) main.o visibility.o -o threaded.exe

main.o: main.cpp visibility.hpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c main.cpp -o main.o

visibility.o: visibility.cpp visibility.hpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c visibility.cpp -o visibility.o

clean:
	rm -f *.o threaded