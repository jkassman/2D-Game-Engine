#include "Crack.hpp"
#include "JDL.hpp"

#define _USE_MATH_DEFINES

#include <math.h>
#include <algorithm>
#include <iostream>

using namespace std;

Crack::Crack(Shape *parentShape, Point startPoint, Line *startLine)
{
    this->shapeSplit = false;
    this->startLine = startLine;
    this->parentShape = parentShape;
    addPoint(startPoint);
}

Crack::Crack(const Crack &other)
{
    //TODO!
}

void Crack::increase(double force)
{
    if (shapeSplit)
    {
        cerr << "Cannot increase crack that has already split the shape!"
             << endl;
        return;
    }

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
    int turnDegrees = 110;

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
            double direction = lines.back()->getDirection();
            int turnDegrees = 110;
            int modifier = rand() % turnDegrees;
            modifier -= turnDegrees/2;
            //modifier = 25; //temporary debug lock
            degree = direction + modifier;
        }


        fractureLine = Line(lines.back()->point2, force, degree);
        JDL::setDrawColor(255, 0, 0);
        fractureLine.draw();
        JDL::setDrawColor(255,255,255);
        JDL::flush();
        cout << "parentShape's size: " << parentShape->lines.size() << endl;

        //if it intersects with a crack, choose a new line.
        vector<Line*>::iterator l;
        vector<Crack*>::iterator c;
        //for all lines
        bool badLine = false;
        for (l = parentShape->lines.begin(); l != parentShape->lines.end(); ++l)
        {
            for (c = (*l)->cracks.begin(); c != (*l)->cracks.end(); ++c)
            {
                if ((*c) == this) break; //exempt self intersection for now.
                if ((*c)->lineIntersects(fractureLine))
                {
                    badLine = true;
                }
            }
        }
        if (badLine) 
        {
            turnDegrees += 10;
            if (turnDegrees > 360)
            {
                turnDegrees = 360;
            }
            continue;
        }

        //for all cracks on those lines
        //for all lines on those cracks

        //see how many lines fractureLine's ray intersects with.
        //(make sure the crack is in the right direction)
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
    //But we need to count how many shape lines it intersects
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
        shapeSplit = true;
    }
    addPoint(fractureLine.point2);
    cout << "exiting increase()" << endl;
    JDL::setDrawColor(0, 255, 0);
    fractureLine.draw();
    JDL::setDrawColor(255,255,255);
    JDL::flush();
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


void Crack::clearLines()
{
    lines.clear();
}

//splitLines[0] is the startLine of the shape
//splitLines.back() is the endLine of the shape
void Crack::getSplitLines(vector<Line*> *splitLines)
{
    splitLines->push_back(startLine);
    vector<Line*>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        splitLines->push_back(*i);
    }

    //discover the endLine
    Point endPoint = lines.back()->point2;
    //For now, assumes the last point in lines is on shape
    for (i = parentShape->lines.begin(); i != parentShape->lines.end(); ++i)
    {
        if ((*i)->on(endPoint))
        {
            splitLines->push_back(*i);
        }
    }
}

bool Crack::isShapeSplit()
{
    return shapeSplit;
}

void Crack::setParent(Shape *parentShape)
{
    this->parentShape = parentShape;
}

Line *Crack::getStartLine()
{
    return this->startLine;
}

bool Crack::lineIntersects(const Line &toCheck)
{
    vector<Line*>::iterator l;
    vector<Crack*>::iterator c;
    Point dummy;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l)->intersects(toCheck, &dummy))
        {
            return true;
        }
        for (c = (*l)->cracks.begin(); c != (*l)->cracks.end(); ++c)
        {
            if ((*c)->lineIntersects(toCheck))
            {
                return true;
            }
        }
    }
    return false;
}
