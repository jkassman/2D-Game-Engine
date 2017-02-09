#include "Crack.hpp"

#define _USE_MATH_DEFINES

#include <math.h>
#include <algorithm>
#include <iostream>

using namespace std;

Crack::Crack(Shape *parent, Point startPoint, Line *startLine)
{
    parentShape = parent;
    this->startLine = startLine;
    addPoint(startPoint);
    splitting = false;
}

void Crack::increase(double force)
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
    vector<Line> intersectLines;
    Line fractureLine;

    while (true)
    {
        //create a random slope of the line
        int degree = rand() % 360;
        //double slope = tan(degree*M_PI / 180);
        Point startPoint = lines.back().point2;
        Point nextPoint(startPoint.x + cos(degree*M_PI / 180) * force, 
                        startPoint.y + sin(degree*M_PI / 180) * force);
        fractureLine = Line(startPoint, nextPoint);

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
    vector<Line>::iterator i;
    Point intersectPoint;
    for (i = parentShape->lines.begin(); i != parentShape->lines.end(); ++i)
    {
        if (i->intersects(fractureLine, &intersectPoint))
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
    
    Line shapeLine;

    if (numIntersects > 0)
    {
        fractureLine.point2 = intersectPoints[0];
        shapeLine = intersectLines[0];
        if (numIntersects > 1)
        {
            //if there are multiple intersects, choose the closest edge
            vector<Point>::iterator j;
            vector<Line>::iterator shapeLineIter;
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
    }
    //lines[0].point2.print();
    addPoint(fractureLine.point2);
    //lines[0].point2.print();

    if (numIntersects != 0)
    {
        split(&shapeLine);
    }
    //draw(); //TODO: consider whether or not this should be here
}

void Crack::move(double distance, double degrees)
{
    vector<Line>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        i->move(distance, degrees);
    }
}

void Crack::draw() const
{
    vector<Line>::const_iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        i->draw(); //oo this looks like a possible stack overflow :)
        //(Look at Line's draw. it calls this draw).
    }
}

Point Crack::startPoint()
{
    return lines[0].point1;
}

void Crack::addPoint(Point toAdd)
{
    cout << "size: " << lines.size() << endl;
    if (!lines.size())
    {
        point = true;
        lines.push_back(Line(toAdd, toAdd));
    }
    else if (point)
    {
        lines[0].point2 = toAdd;
        point = false;
    }
    else
    {
        lines.push_back(Line(lines.back().point2, toAdd));
    }
    (lines.end() - 1)->index = lines.size()-1;
}

//the crack should be destroyed after a split
void Crack::split(Line *endLine)
{
    //soo...the main problem might be we need to remove this crack from
    //line's cracks. Which will leave this dangling.
    //nah, let's just set a variable in here.
    splitting = true;

    //alright, so what to do here?
    //we have the start and end points of the crack...
    //so what to do..
    //find what line the end point of the crack is on? Well..
    //we could be passed that.
    //I mean I could check for equality..or be passed the index..
    //right now equality is probably easier. Could be improved for speed later

    //ok. so..we search the shape until we find a line equal to this one
    //then we mark this as the start point, the next point is startLine.point2.
    //then..we continue on the line until we reach endLine
    //at which point...we cap it with endPoint.


    //new plan: Copy the crack's point vector.
    //Start at the endPoint of the crack.
    //go to endLine's point2.
    //walk down the shape's line until we hit startLine
    //then add the startpoint--no don't. That will define a shape.
    //so there's one shape.

    //do the same thing expect going down endline's point1?

    


    //new new plan:
    //split the shape by lines, create two new shapes from them.
    //split startLine, split endLine, then throw the lines into shapes.
    
    //endLine splits intersectPoint->point2.
    Line start1, start2;
    int start1Index, start2Index;
    vector<Line> lines1, lines2;
    
    start1Index = endLine->index;
    endLine->split(lines.back().point2, &start1);
    //endLine is now endLine.point1->intersectPoint
    
    start2Index = startLine->index;
    startLine->split(lines[0].point1, &start2);
    //startLine is now startLine.point1->intersectPoint
    
    start1.index = 0;
    start2.index = 0;
    int newIndex = 1;

    lines1.push_back(start1);
    lines2.push_back(start2);

    //now we want all lines in the shape between start1 and startLine.
    //let's assume indexes work.
    //cout << "start indexes" << start1Index << "," << start2Index << endl;
    
    vector<Line>::iterator i = parentShape->lines.begin() + start1Index;
    //Note: i++ works because shape is constructed point1->point2->point1 etc.
    cout << "Pre first loop" << endl;
    lines[0].point2.print();
    cout << start2Index << endl;
    //This should grab ~half the lines in the shape
    while (i != (parentShape->lines.begin() + start2Index))
    {
        ++i;
        if(i == parentShape->lines.end())
        {
            i = parentShape->lines.begin();
        }
        i->index = newIndex;
        newIndex++;

        lines1.push_back(*i);
        //i = parentShape->lines.erase(i);
    }
    cout << "Pre second loop, line size: " << lines.size() << endl;
    //This copies all the lines in the crack
    vector<Line>::iterator j;
    
    for (j = lines.begin(); j != lines.end(); ++j)
    {
        j->index = newIndex;
        newIndex++;
        lines1.push_back(*j);
    }
    cout << "Shape creation" << endl;
    //ok, now create a shape out of all that. Move it away, too.
    //parentShape->allShapes->push_back(Shape(lines1, parentShape->allShapes));
    //parentShape->allShapes->back().move(150, 0);
    Shape newShape(lines1, parentShape->allShapes);
    newShape.move(150, 0);
    newShape.draw();
    cout << "Finished the split" << endl;
}

bool Crack::isSplitting() const
{
    return splitting;
}
