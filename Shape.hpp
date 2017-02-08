#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "Line.hpp"
#include "Crack.hpp"
#include "gfx_j.h"

#include <vector>

class Crack;
class Line;

//A shape is just a collection of LINES
class Shape
{
    friend class Crack;

private:
    std::vector<Line> lines;
    std::vector<Shape> *allShapes; //pointer to a vector of all drawable shapes.

public:
    Shape(std::vector<Point> givenPoints, std::vector<Shape> *allShapes);
    Shape(std::vector<Line> &givenLines, std::vector<Shape> *allShapes);
    //Whether or not the given point is inside this shape
    bool inside(Point toTest);
    
    void move(double distance, double degrees);
    void draw();

    int rayTrace(Line &ray);

    void addPoint(Point toAdd);
    void fractureAt(Point clickPoint);
};

#endif
