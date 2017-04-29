CC_FLAGS = -Wall -Werror -ggdb

main: Point.o Shape.o Line.o Crack.o main.cpp gfx_j.o JDL.o Launcher.o jacobJSON.o
	g++ $^ -o main $(CC_FLAGS) -lX11

test: tester.cpp Line.o gfx_j.o JDL.o Point.o Crack.o Shape.o tests.o
	g++ $^ -o tester $(CC_FLAGS) -lX11 

testOn: testOn.cpp Line.o gfx_j.o JDL.o Point.o Crack.o Shape.o tests.o
	g++ $^ -o testOn $(CC_FLAGS) -lX11

gfx_j.o: gfx_j.c gfx_j.h
	g++ gfx_j.c -c 

%.o: %.cpp %.hpp
	g++ -c $< -o $@ $(CC_FLAGS)

clean:
	rm *.o main
