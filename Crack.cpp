#include "Crack.hpp"

#include <math.h>

using namespace std;

Crack::Crack(Shape *parent, Point startPoint, Line startLine)
{
    parentShape = parent;
    line = startLine;
    points.push_back(startPoint);
}

void Crack::increase(int force)
{
    //ideas: create a line with a random slope starting from the given point.
    //check all lines in the shape to see if any intersect
    //(disregard the line the fracture point is on)
    //if multiple lines intersect, pick the closest line
    //if no lines intersect, choose a new random line

    //ok, so how to create a random slope?
    //we should pick a random angle...then take sine or cosine or something?
    rand()
}
