#include "Shape.hpp"
#include "JDL.h"

#include <iostream>

using namespace std;

Shape::Shape(vector<Point> givenPoints, vector<Shape> *allShapes)
{
    this->points = givenPoints;
    this->shapes = allShapes;
	generateLines();
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
		i->draw();
    }
}

void Shape::fractureAt(Point clickPoint)
{
    vector<Line>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        Point result;
		if (i->pointWithin(clickPoint, 10, &result))
        {
            //JDL::circle(result.x, result.y, 8);
            cracks.push_back(Crack(this, result, *i));
            (cracks.end()-1)->increase(10);
            return;
        }
    }
}

void Shape::addPoint(Point otherPoint)
{
	points.push_back(otherPoint);
	generateLines();
}

void Shape::generateLines()
{
	lines.clear();
	vector<Point>::iterator i;
	vector<Point>::iterator next;
	for (i = this->points.begin(); i != this->points.end(); ++i)
	{
		if ((i + 1) == this->points.end())
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
