CXX=g++
INCLUDE=-I"../Base/" -I"../Decompiler/" -I"../Debugger/" -I"../GBA/" -I"../libs/include"
LD=g++
LIBS=-L$(shell pwd)/../bin -L"../libs/lib" -lDebugger -lDecompiler -lBase -lSDL2 -lSDL2_ttf
OBJS=GBA/tests.o GBA/main.o

all: ../bin/gba

../bin/gba: $(OBJS)
	$(LD) -O3 -o ../bin/gba $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) -c -O3 -std=c++11 $(INCLUDE) -o $*.o $<

clean:
	rm -rf $(OBJS)
	rm -f ../bin/gba
