CXX=g++
INCLUDE=-I"../Base/" -I"../Decompiler/" -I"../libs/include/"
OBJS=debugger/Debugger.o debugger/CLI.o debugger/GUI.o debugger/VRAM_GUI.o interpreter/Interpreter.o interpreter/ARM/ARMBranch.o interpreter/ARM/ARMDataProcessing.o interpreter/ARM/ARMInstructions.o interpreter/ARM/ARMLoadStore.o interpreter/THUMB/THUMBALU.o interpreter/THUMB/THUMBDataProcessing.o interpreter/THUMB/THUMBInstructions.o
LIBS=-L$(shell pwd)/../bin -L"../libs/lib" -lDecompiler -lBase -lSDL2 -lSDL2_ttf

all: ../bin/libDebugger.so

../bin/libDebugger.so: $(OBJS)
	$(CXX) -shared -O3 $(OBJS) -o ../bin/libDebugger.so $(LIBS)
	
%.o: %.cpp
	$(CXX) -c -fPIC -O3 -std=c++11 $(INCLUDE) -o $*.o $<

clean:
	rm -rf $(OBJS)
	rm -f ../bin/libDebugger.so
