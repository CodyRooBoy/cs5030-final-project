CXX = g++
NVCC = nvcc
CXXFLAGS = -std=c++17
NVCCFLAGS = -std=c++17

TARGET = distributed_gpu
OBJS = main.o visibility_cuda.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(NVCC) $(OBJS) -o $(TARGET)

main.o: main.cpp visibility_cuda.o
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

visibility_cuda.o: visibility_cuda.cu visibility_cuda.hpp
	$(NVCC) $(NVCCFLAGS) -c visibility_cuda.cu -o visibility_cuda.o

clean:
	rm -f *.o $(TARGET)