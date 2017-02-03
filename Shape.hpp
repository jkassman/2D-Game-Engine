#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "Line.hpp"
#include "Crack.hpp"
#include "gfx_j.h"

#include <vector>

class Crack;

class Shape
{
	friend class Crack;

private:
    //Assumed all points are connected to each other (with lines) in order.
    //For example, 0-1, 1-2, and 2-0 all have lines connecting them. 
    std::vector<Point> points;
    std::vector<Line> lines; //lines are generated from points; used to draw
    //they are updated whenever points change.
    std::vector<Crack> cracks;
    std::vector<Shape> *shapes; //pointer to a vector of all drawable shapes.

	void generateLines();

public:
    Shape(std::vector<Point> givenPoints, std::vector<Shape> *allShapes);

    //Whether or not the given point is inside this shape
    bool inside(Point toTest);
    
    void fractureAt(Point clickPoint);

    void draw();

    int rayTrace(Line &ray);

    void addPoint(Point otherPoint);
};

#endif
