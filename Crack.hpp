#ifndef CRACK_HPP
#define CRACK_HPP

#include "Point.hpp"
#include "Shape.hpp"

#include <vector>

class Shape;

class Crack
{
private:
    std::vector<Point> points;
    Line line;
    Shape *parentShape;

    //modifies the current shape and creates a new one.
    //called by increase when the crack intersects a line
    void split();

public:
    Crack(Shape *parent, Point startPoint, Line startLine);
    
    //force is a measure of how much the crack should increase in length
    void increase(int force);

    void draw();
};

#endif
