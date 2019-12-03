CXX=g++
INCLUDE=
OBJS=JIT.o

all: jit_test

jit_test: $(OBJS)
	$(CXX) $(OBJS) -o jit_test
	
%.o: %.cpp
	$(CXX) -c -fPIC $(INCLUDE) -o $*.o $<

clean:
	rm -rf $(OBJS)
	rm jit_test