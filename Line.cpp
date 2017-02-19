#include "Line.hpp"
#include "JDL.hpp"

#define _USE_MATH_DEFINES

#include <algorithm>
#include <iostream>
#include <math.h>
#include <string>

#include <stdio.h>

using namespace std;

//#define CWR 2

Line::Line()
{

}

Line::Line(Point point1, Point point2)
{
    this->point1 = point1;
    this->point2 = point2;
}

Line::Line(Point point1, double length, double direction)
{
    this->point1 = point1;
    this->point2 = Point(point1.x + cos(direction*M_PI / 180) * length, 
                         point1.y + sin(direction*M_PI / 180) * length);
}

Line::Line(const Line & other)
{
    this->point1 = other.point1;
    this->point2 = other.point2;
    this->index = other.index;

    vector<Crack*>::iterator i;
    for (i = cracks.begin(); i != cracks.end(); ++i)
    {
        this->cracks.push_back(new Crack(**i));
    }  
}

void Line::move(double distance, double degrees)
{
    point1.x += cos(degrees/180*M_PI)*distance;
    point1.y += sin(degrees/180*M_PI)*distance;
    point2.x += cos(degrees/180*M_PI)*distance;
    point2.y += sin(degrees/180*M_PI)*distance;
    vector<Crack*>::iterator i;
    for (i = cracks.begin(); i != cracks.end(); ++i)
    {
        (*i)->move(distance, degrees);
    }
}

#ifndef JDL_USE_SDL
string to_string(int lazy)
{
    char temp[100];
    sprintf(temp, "%d", lazy);
    return string(temp);
}
#endif

void Line::draw() const
{
    JDL::line(JDL::roundi(point1.x), JDL::roundi(point1.y),
              JDL::roundi(point2.x), JDL::roundi(point2.y));
/*
    double dx = point2.x - point1.x;
    double dy = point2.y - point1.y;
    JDL::text(JDL::roundi(point1.x + dx/2), 
              JDL::roundi(point1.y + dy/2), 
              to_string(index).c_str());
*/
    vector<Crack*>::const_iterator i;
    for (i = cracks.begin(); i != cracks.end(); ++i)
    {
        (*i)->draw();
    }
}

double Line::length() const
{
    return sqrt(pow(point1.x-point2.x, 2) + pow(point1.y - point2.y, 2));
}

//check if the two lines (not segments) intersect
bool Line::intersectsInfinite(const Line &otherLine, Point *resultPoint)
{
    double x1, x2, x3, x4;
    double y1, y2, y3, y4;
    x1 = point1.x;
    x2 = point2.x;
    x3 = otherLine.point1.x;
    x4 = otherLine.point2.x;
    y1 = point1.y;
    y2 = point2.y;
    y3 = otherLine.point1.y;
    y4 = otherLine.point2.y;
    double numX, denomX;
    numX = (x1*y2 - y1*x2)*(x3-x4) - (x1 - x2)*(x3*y4 - y3*x4);
    denomX = (x1 -x2)*(y3 - y4) - (y1 - y2)*(x3 - x4);
    double numY, denomY;
    numY = (x1*y2 - y1*x2)*(y3 - y4) - (y1 - y2)*(x3*y4 - y3*x4);
    denomY = (x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4);
    if (JDL::doublesEqual(denomX, 0) || JDL::doublesEqual(denomY, 0))
    {
        return false;
    }
    *resultPoint = Point(numX/denomX, numY/denomY);
    return true;
}

//this line is the ray, otherLine is a segment
//return true if they intersect, false otherwise.

/* Probably isn't working, requires further testing */
bool Line::rayIntersects(const Line &otherLine, Point *resultPoint)
{
    Point intersect;
    if (!intersectsInfinite(otherLine, &intersect))
    {
        return false;
    }
    if (!otherLine.on(intersect))
    {
        return false;
    }
    //we now know that this infinite line intersects with the otherLine segment.
    //Now we need to know if this intersection was on the wrong side of the ray.

    if (point2.x > point1.x)
    { //then decreasing x is bad
        if (intersect.x < point1.x - JDL::PRECISION)
        {
            return false;
        }
    }
    else
    { //increasing x is bad
        if (intersect.x > point1.x + JDL::PRECISION)
        {
            return false;
        }
    }
    
    if (point2.y > point1.y)
    { //decreasing y is bad
        if (intersect.y < point1.y - JDL::PRECISION)
        {
            return false;
        }
    }
    else
    { //increasing y is bad
        if (intersect.y > point1.y + JDL::PRECISION)
        {
            return false;
        }
    }
    
    //cout << "VALID!" << endl;
    //JDL::circle(intersect.x, intersect.y, 10);
    *resultPoint = intersect;
    return true;
}

//check if the two line segments intersect
bool Line::intersects(const Line &otherLine, Point *resultPoint)
{
    Point temp;
    if (!intersectsInfinite(otherLine, &temp))
    {
        return false;
    }
    if (this->on(temp) && otherLine.on(temp))
    {
        *resultPoint = temp;
        return true;
    }
    return false;
}

bool Line::on(Point testPoint, double radius, Point *resultPoint) const
{
    double slope;
    double maxY, minY, maxX, minX;
    maxY = max(point2.y, point1.y);
    minY = min(point2.y, point1.y); 
    maxX = max(point2.x, point1.x);
    minX = min(point2.x, point1.x);
    
    if ((testPoint.y <= (maxY + radius))
        && (testPoint.y >= (minY- radius))
        && (testPoint.x <= (maxX + radius)) 
        && (testPoint.x >= (minX - radius)))
    {
        if (JDL::doublesEqual(point2.x, point1.x))
        { //catch case where line is vertical
            double verticalY;
            if (testPoint.y > maxY)
            {
                verticalY = maxY;
            }
            else if (testPoint.y < minY)
            {
                verticalY = minY;
            }
            else
            {
                verticalY = testPoint.y;
            }
            resultPoint->x = point2.x;
            resultPoint->y = verticalY;
            return true;
        }  
        else
        {
            slope = (point2.y - point1.y) / (point2.x - point1.x);
            double checkY = (slope*testPoint.x + point1.y - (slope * point1.x));
            if ((testPoint.y >= (checkY - radius))
                && (testPoint.y <= (checkY + radius)))
            {
                double returnX, returnY;
                if (testPoint.x > maxX)
                {
                    returnX = maxX;
                }
                else if (testPoint.x < minX)
                {
                    returnX = minX;
                }
                else
                {
                    returnX = testPoint.x;
                }
                if (checkY > maxY)
                {
                    returnY = maxY;
                }
                else if (checkY < minY)
                {
                    returnY = minY;
                }
                else
                {
                    returnY = checkY;
                }
                resultPoint->x = returnX;
                resultPoint->y = returnY;
                return true; 
            }
        }
    }
    return false;
}

//lazy implementation
bool Line::on(Point testPoint) const
{
    Point dummyPoint;
    return this->on(testPoint, JDL::PRECISION, &dummyPoint);
}

bool Line::operator==(const Line &other)
{
    return ((point1 == other.point1) && (point2 == other.point2));
}

/*
Line Line::operator=(const Line &other)
{
    point1 = other.point1;
    point2 = other.point2;
    index = other.index;
    cracks = other.cracks;
    return *this;
}

void Line::createFracture(Point startPoint, Shape *parentShape, double force)
{
    cracks.push_back(Crack(parentShape, startPoint, this));
    cracks.back().increase(force);
    }
*/
void Line::getImpactedCracks(Point impactPoint, Shape *crackParentShape, double force,
                             std::vector<Crack*> *impactedCracks)
{
    vector<Crack*>::iterator i;
    double radius = 10; //eventually, set the radius based on the force.
    for (i = cracks.begin(); i != cracks.end(); ++i)
    {
        if ((*i)->startPoint().near(impactPoint, radius))
        {

        }
    }
    while (i != cracks.end())
    {
        if ((*i)->startPoint().near(impactPoint, 10))
        {
            numCracksIncreased++;
            (*i)->increase(force);
            if ((*i)->isShapeSplit())
            {
                (*i)->getSplitLines(&splitLines);
                (*i)->clearLines(); //so the destructor doesn't erase the lines
                i = cracks.erase(i);
                parentShape->split(splitLines);
                return 1;
            }
            else
            {
                ++i;
            }
        }
        else
        {
            ++i;
        }
    }
    if (numCracksIncreased == 0)
    {
        cracks.push_back(new Crack(parentShape, impactPoint, this));
        cracks.back()->increase(force);
        if (cracks.back()->isShapeSplit())
        {
            cracks.back()->getSplitLines(&splitLines);
            cracks.back()->clearLines();
            cracks.pop_back();
            parentShape->split(splitLines);
            return 1;
        }

    }
    cout << "Finished increasing cracks" << endl;
    return numSplit;
}

}

             
int Line::increaseCracks(Point impactPoint, Shape *parentShape, double force)
{
    //eventualy, want to handle multiple splits.

    //eventually, increase all cracks that are near.

    //for now, incrase any crack within a certain radius.
    int numCracksIncreased = 0;
    vector<Crack*>::iterator i = cracks.begin();
    vector<Line*> splitLines;
    int numSplit = 0;
    while (i != cracks.end())
    {
        if ((*i)->startPoint().near(impactPoint, 10))
        {
            numCracksIncreased++;
            (*i)->increase(force);
            if ((*i)->isShapeSplit())
            {
                (*i)->getSplitLines(&splitLines);
                (*i)->clearLines(); //so the destructor doesn't erase the lines
                i = cracks.erase(i);
                parentShape->split(splitLines);
                return 1;
            }
            else
            {
                ++i;
            }
        }
        else
        {
            ++i;
        }
    }
    if (numCracksIncreased == 0)
    {
        cracks.push_back(new Crack(parentShape, impactPoint, this));
        cracks.back()->increase(force);
        if (cracks.back()->isShapeSplit())
        {
            cracks.back()->getSplitLines(&splitLines);
            cracks.back()->clearLines();
            cracks.pop_back();
            parentShape->split(splitLines);
            return 1;
        }
        
    }
    cout << "Finished increasing cracks" << endl;
    return numSplit;
}

//splitPoint must be on the line
//will create two new lines: this will be point1->splitPoint,
//newLine is splitPoint->point2
void Line::split(Point splitPoint, Line *newLine)
{
    if (!on(splitPoint))
    {
        cerr << "The point to split is not on the line!" << endl;
        return;
    }
    newLine->index = index;
    newLine->point1 = splitPoint;
    newLine->point2 = point2;

    point2 = splitPoint;
    //split up the cracks:
    vector<Crack*>::iterator i = cracks.begin();
    while (i != cracks.end())
    {
        if (on((*i)->startPoint()))
        {
            ++i;
        }
        else
        {
            newLine->cracks.push_back(*i);
            i = cracks.erase(i);
        }
    }
}


void Line::switchPoints()
{
    Point temp = point1;
    point1 = point2;
    point2 = temp;
}

double Line::getDirection()
{
    double direction;
    direction = JDL::calculateTheta(point2.x-point1.x, point2.y-point1.y);
    direction *= 180/M_PI;
    direction *= -1; //not sure why this is needed...
    return direction;
}
