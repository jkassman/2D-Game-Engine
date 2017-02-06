#include "Shape.hpp"
#include "JDL.hpp"

#include <iostream>
#include <algorithm>

using namespace std;

Shape::Shape(vector<Point> givenPoints, vector<Shape> *allShapes)
{
    this->points = givenPoints;
    this->shapes = allShapes;
    generateLinesFromPoints(&lines, points);
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
		if (i->on(clickPoint, 10, &result))
        {
            //JDL::circle(result.x, result.y, 8);
            cracks.push_back(Crack(this, result, *i));
            (cracks.end()-1)->increase(100);
            return;
        }
    }
}

void Shape::addPoint(Point otherPoint)
{
	points.push_back(otherPoint);
	generateLinesFromPoints(&lines, points);
}

void generateLinesFromPoints(vector<Line> *lines, vector<Point> &points)
{
    lines->clear();
    vector<Point>::iterator i;
    vector<Point>::iterator next;
    for (i = points.begin(); i != points.end(); ++i)
    {
        if ((i + 1) == points.end())
        {
            next = points.begin();
        }
        else
        {
            next = i + 1;
        }
        lines->push_back(Line(*i, *next));
    }
}


//returns the number of lines in the shape that are intersected.
//Assumes the line is a ray starting at point1 and going to point2.
int Shape::rayTrace(Line &ray)
{
    //check to see how many lines in the shape the ray intersects with
    Point intersectPoint;
    vector<Line>::iterator i;
    int numIntersects = 0;
    vector<Point> intersectPoints;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        if (ray.rayIntersects(*i, &intersectPoint))
        {
            intersectPoints.push_back(intersectPoint);
            numIntersects++;
        }
    }
    
    //check for duplicates:
    sort(intersectPoints.begin(), intersectPoints.end());
    vector<Point>::iterator j;
    Point previous = *intersectPoints.begin();
    for (j = intersectPoints.begin()+1; j != intersectPoints.end(); ++j)
    {
        if (previous == *j)
        {
            cout << "Found a duplicate!" << endl;
            numIntersects--;
        }
        previous = *j;
    }

    return numIntersects;
}
