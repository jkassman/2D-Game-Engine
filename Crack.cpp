#include "Crack.hpp"
#include "JDL.hpp"

#define _USE_MATH_DEFINES

#include <math.h>
#include <algorithm>
#include <iostream>

using namespace std;

Crack::Crack(Shape *parentShape, Point startPoint, Line *startLine)
{
    this->startLine = startLine;
    this->parentShape = parentShape;
    addPoint(startPoint);
}

Crack::Crack(const Crack &other)
{
    //TODO!
}

int Crack::increase(double force)
{
    //New Idea:
    //Create lines with random slopes starting from the given point until
    //One of those lines points towards the inside of the shape
    
    //Then, see if that line intersects with the edge of the shape
    //    (Or maybe even another crack)
    //If it does, split the shape apart.
    //    (If it intersects with multiple, choose the closest edge)

    int numIntersects = 0;
    bool startsOnLine = false;
    
    vector<Point> intersectPoints;
    vector<Line*> intersectLines;
    Line fractureLine;

    while (true)
    {
        int degree;
        //create a random slope of the line
        if (point)
        {
            degree = rand() % 360;
        }
        else
        {
            int modifier = rand() % 90;
            modifier -= 45;
            //modifier = 10; //temporary debug lock
            degree = lines.back()->getDirection() + modifier;
        }
        //double slope = tan(degree*M_PI / 180);
        //Point startPoint = lines.back()->point2;
        //Point nextPoint(startPoint.x + cos(degree*M_PI / 180) * force, 
        //                startPoint.y + sin(degree*M_PI / 180) * force);
        fractureLine = Line(lines.back()->point2, force, degree);

        //see how many lines fractureLine's ray intersects with.
        int numRay = parentShape->rayTrace(fractureLine);
        cout << "rayTrace returned: " << numRay;
        //if the start point of this fracture is on the crack's start line,
        //subtract one from the rayTrace count.
        if (startLine->on(fractureLine.point1))
        {
            startsOnLine = true;
            numRay--;
            cout << ", corrected to: " << numRay;
        }
        cout << endl;

        //if the number lines intersected is odd, it's in the right direction.
        //else, repeat this process again.
        if (numRay % 2) //if odd
        {
            break;
        }
    }
    //Now we know the crack is in the right direction.
    //But we need to count how many lines it intersects
    //(if it intersects no lines, we're done)
    vector<Line*>::iterator i;
    Point intersectPoint;
    for (i = parentShape->lines.begin(); i != parentShape->lines.end(); ++i)
    {
        if ((*i)->intersects(fractureLine, &intersectPoint))
        {
            if (startsOnLine)
            {
                //disregard intersects on the line the crack starts from
                if (startLine->on(intersectPoint))
                {
                    continue;
                }
            }
            numIntersects++;
            intersectPoints.push_back(intersectPoint);
            intersectLines.push_back(*i);
        }
    }
    
    Line *shapeLine;

    if (numIntersects > 0)
    {
        fractureLine.point2 = intersectPoints[0];
        shapeLine = intersectLines[0];
        if (numIntersects > 1)
        {
            //if there are multiple intersects, choose the closest edge
            vector<Point>::iterator j;
            vector<Line*>::iterator shapeLineIter = intersectLines.begin();
            for (j = intersectPoints.begin()+1; j != intersectPoints.end(); ++j)
            {
                shapeLineIter++;
                Line checkLine(fractureLine.point1, *j);
                if (checkLine.length() < fractureLine.length())
                {
                    fractureLine.point2 = *j;
                    shapeLine = *shapeLineIter;
                }
            }
        }
        addPoint(fractureLine.point2);
        split(shapeLine);
        cout << "Exiting increase()" << endl;
        return 1;
    }
    else
    {
        addPoint(fractureLine.point2);
    }
    return 0;
}

void Crack::move(double distance, double degrees)
{
    vector<Line*>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        (*i)->move(distance, degrees);
    }
}

void Crack::draw() const
{
    vector<Line*>::const_iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        (*i)->draw(); //oo this looks like a possible stack overflow :)
        //(Look at Line's draw. it calls this draw).
    }
}

Point Crack::startPoint()
{
    return lines[0]->point1;
}

void Crack::addPoint(Point toAdd)
{
    //cout << "size: " << lines.size() << endl;
    if (!lines.size())
    {
        point = true;
        lines.push_back(new Line(toAdd, toAdd));
    }
    else if (point)
    {
        lines[0]->point2 = toAdd;
        point = false;
    }
    else
    {
        lines.push_back(new Line(lines.back()->point2, toAdd));
    }
    lines.back()->index = lines.size()-1;
}


void Crack::split(Line *endLine)
{
    //this function should generate two separate lines from the crack that connects startLine and endLine.
    //The two lines will be identical, at first
    //(When we start having cracks off of cracks, the two sets of lines will differ;
    //    The cracks will not be duplicated, but one set of lines gets one crack
    //    and one set of lines gets another).
    //After that, it calls Shape's split.
    //nevermind all that. The main job of this is to create a vector of lines
    //from one line of the shape to another line of the shape.
    //which is trivial for now.
/*
    //First, remove this crack from the shape's lines:
    vector<Crack*>::iterator i;
    for (i = startLine->cracks.begin(); i != startLine->cracks.end(); ++i)
    {
        if ((*i) == this)
        {
            startLine->cracks.erase(i);
            break;
        }
    }
*/
    //JDL::clear();
    //draw();
    //JDL::flush();
    parentShape->split(startLine, endLine, lines);

    lines.clear();
}
