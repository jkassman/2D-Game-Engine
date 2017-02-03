#include "Crack.hpp"

#define _USE_MATH_DEFINES

#include <math.h>
#include <algorithm>
#include <iostream>

using namespace std;

Crack::Crack(Shape *parent, Point startPoint, Line startLine)
{
    parentShape = parent;
    line = startLine;
    points.push_back(startPoint);
}

void Crack::increase(int force)
{
    //New Idea:
    //Create lines with random slopes starting from the given point until
    //One of those lines points towards the inside of the shape
    
    //Then, see if that line intersects with the edge of the shape
    //    (Or maybe even another crack)
    //If it does, split the shape apart.
    //    (If it intersects with multiple, choose the closest edge)

    int radius = force * 100;
    int numIntersects = 0;
    vector<Point> intersectPoints;
    Line fractureLine;

    while (numIntersects == 0)
    {
        //create a random slope of the line
        int degree = rand() % 360;
        //double slope = tan(degree*M_PI / 180);
        Point nextPoint(points[0].x + cos(degree*M_PI / 180) * radius, 
                        points[0].y + sin(degree*M_PI / 180) * radius);
        fractureLine = Line(points[0], nextPoint);

        //see how many lines fractureLine's ray intersects with.
        int numRay = parentShape->rayTrace(fractureLine);
        cout << "rayTrace returned: " << numRay;
        //if the start point of this fracture is on the crack's start line,
        //subtract one from the rayTrace count.
        bool startsOnLine = false;
        if (line.on(fractureLine.point1))
        {
            startsOnLine = true;
            numRay--;
            cout << ", corrected to: " << numRay;
        }
        cout << endl;

        //if the number lines intersected is odd, it's in the right direction.
        //else, choose a new fracture line (skip to the beginning of the loop.
        if ((numRay % 2) == 0)
        {
            continue;
        }

        //so now we know the crack is in the right direction.
        //But we need to see where it intersects with any line in the shape.
        //    (If it doesn't intersect, we're done). 

        //check if the fractureLine intersects with any lines in the shape
        vector<Line>::iterator i;
        Point intersectPoint;
        for (i = parentShape->lines.begin(); i != parentShape->lines.end(); ++i)
        {
            if (i->intersects(fractureLine, &intersectPoint))
            {
                if (startsOnLine)
                {
                    //disregard intersects on the line the crack starts from
                    if (line.on(intersectPoint))
                    {
                        continue;
                    }
                }
                numIntersects++;
                intersectPoints.push_back(intersectPoint);
            }
        }
    }

    if (numIntersects > 0)
    {
        fractureLine.point2 = intersectPoints[0];
        if (numIntersects > 1)
        {
            //if there are multiple intersects, choose the closest edge
            vector<Point>::iterator j;
            for (j = intersectPoints.begin()+1; j != intersectPoints.end(); ++j)
            {
                Line checkLine(fractureLine.point1, *j);
                if (checkLine.length() < fractureLine.length())
                {
                    fractureLine.point2 = *j;
                }
            }
        }
    }

    fractureLine.draw();
}
