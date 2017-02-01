#include "Shape.hpp"
#include "gfx_j.h"

#include <iostream>

using namespace std;

Shape::Shape(vector<Point> givenPoints, vector<Shape> *allShapes)
{
    this->points = givenPoints;
    this->shapes = allShapes;

    vector<Point>::iterator i;
    vector<Point>::iterator next;
    for (i = this->points.begin(); i != this->points.end(); ++i)
    {
        if ((i+1) == this->points.end())
        {
            next = this->points.begin();
        }
        else
        {
            next = i + 1;
        }
        this->lines.push_back(Line(Point(i->x, i->y), Point(next->x, next->y)));
    }
}

bool Shape::inside(Point toTest)
{
    return false;
}

void Shape::draw()
{
    vector<Line>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        gfx_line(i->point1.x, i->point1.y,
                 i->point2.x, i->point2.y);
    }
}

void Shape::fractureAt(Point clickPoint)
{
    vector<Line>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        Point result;
        result = i->pointWithin(clickPoint, 10); 
        if (result.x > 0)
        {
            gfx_circle(result.x, result.y, 8);
            cracks.push_back(Crack(this, result, *i));
            (cracks.end()-1)->increase(1);
            return;
        }
    }
}
