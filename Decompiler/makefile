CXX=g++
INCLUDE=-I "../Base/" -I"../libs/include/"
OBJS=ARM/ARMDecompiler.o THUMB/THUMBDecompiler.o
LIBS=-L$(shell pwd)/../bin -L"../libs/lib" -lBase -lSDL2 -lSDL2_ttf

all: ../bin/libDecompiler.so

../bin/libDecompiler.so: $(OBJS)
	$(CXX) -shared -O3 $(OBJS) -o ../bin/libDecompiler.so $(LIBS)
	
%.o: %.cpp
	$(CXX) -c -O3 -fPIC -std=c++11 $(INCLUDE) -o $*.o $<

clean:
	rm -rf $(OBJS)
	rm -f ../bin/libDecompiler.so
