CC_FLAGS = -Wall -Werror

main: Point.o Shape.o Line.o Crack.o main.cpp gfx_j.o
	g++ $^ -o main $(CC_FLAGS) -lX11

gfx_j.o: gfx_j.c gfx_j.h
	g++ gfx_j.c -c 

%.o: %.cpp %.hpp
	g++ -c $< -o $@ $(CC_FLAGS)

clean:
	rm *.o main *~
