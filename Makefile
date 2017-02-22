CC_FLAGS = -Wall -Werror -ggdb

main: Point.o Shape.o Line.o Crack.o main.cpp gfx_j.o JDL.o
	g++ $^ -o main $(CC_FLAGS) -lX11

gfx_j.o: gfx_j.c gfx_j.h
	g++ gfx_j.c -c 

%.o: %.cpp %.hpp
	g++ -c $< -o $@ $(CC_FLAGS)

clean:
	rm *.o main
