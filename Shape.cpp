#include "Shape.hpp"
#include "JDL.hpp"

#include <iostream>
#include <algorithm>

using namespace std;

Shape::Shape(vector<Point> givenPoints, vector<Shape> *allShapes)
{
    this->allShapes = allShapes;
    vector<Point>::iterator i;
    vector<Point>::iterator next;
    for (i = givenPoints.begin(); i != givenPoints.end(); ++i)
    {
        if ((i + 1) == givenPoints.end())
        {
            next = givenPoints.begin();
        }
        else
        {
            next = i + 1;
        }
        addPoint(*next);
    }
}

Shape::Shape(vector<Line*> &givenLines, vector<Shape> *allShapes)
{
    this->allShapes = allShapes;
    this->lines = givenLines;
}

bool Shape::inside(Point toTest)
{
    return false;
}

void Shape::move(double distance, double degrees)
{
    vector<Line*>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        (*i)->move(distance, degrees);
    }
}

void Shape::draw()
{
    vector<Line*>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        (*i)->draw();
    }
}

void Shape::fractureAt(Point clickPoint)
{
    vector<Line*>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        Point result;
        if ((*i)->on(clickPoint, 10, &result))
        {
            //JDL::circle(result.x, result.y, 8);
            (*i)->createFracture(result, this, 1000);
            return;
        }
    }
}

void Shape::addPoint(Point toAdd)
{
    if (!lines.size())
    {
        lines.push_back(new Line(toAdd, toAdd));
    }
    else
    {
        lines.back()->point2 = toAdd;
        lines.push_back(new Line(toAdd, lines[0]->point1));
    }
    lines.back()->index = lines.size()-1;
}


//returns the number of lines in the shape that are intersected.
//Assumes the line is a ray starting at point1 and going to point2.
int Shape::rayTrace(Line &ray)
{
    //check to see how many lines in the shape the ray intersects with
    Point intersectPoint;
    vector<Line*>::iterator i;
    int numIntersects = 0;
    vector<Point> intersectPoints;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        if (ray.rayIntersects(**i, &intersectPoint))
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
