MPICXX = mpicxx
NVCC = nvcc
LDFLAGS = -lcudart -lstdc++ -llzma
NVCCFLAGS = -Xcompiler -fPIC

TARGET = distributed_gpu
OBJS = main.o visibility_cuda.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(MPICXX) $(LDFLAGS) $(OBJS) -o $(TARGET)

main.o: main.cpp visibility_cuda.o
	$(MPICXX) -c main.cpp -o main.o

visibility_cuda.o: visibility_cuda.cu visibility_cuda.hpp
	$(NVCC) $(NVCCFLAGS) -c visibility_cuda.cu -o visibility_cuda.o

clean:
	rm -f *.o $(TARGET)