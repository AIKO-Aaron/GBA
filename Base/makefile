CXX=g++
INCLUDE=-I"../libs/include" -I"../Base"
OBJS=Base/Logic.o CPU/CPU.o GPU/GPU.o MMU/MMU.o Timers/Timers.o Register/Register.o Register/RegisterSet.o Audio/Audio.o
LIBS=-L$(shell pwd)/../bin -L"../libs/lib" -lSDL2 -lSDL2_ttf

all: ../bin/libBase.so

../bin/libBase.so: $(OBJS)
	$(CXX) -shared -O3 $(OBJS) -o ../bin/libBase.so $(LIBS)
	
%.o: %.cpp
	$(CXX) -c -O3 -std=c++11 -fPIC $(INCLUDE) -o $*.o $<

clean:
	rm -rf $(OBJS)
	rm -f ../bin/libBase.so
