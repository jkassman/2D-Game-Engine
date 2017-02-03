#include "Line.hpp"
#include "JDL.hpp"

#include <algorithm>
#include <iostream>

using namespace std;

#define CWR 2

Line::Line()
{

}

Line::Line(Point point1, Point point2)
{
    this->point1 = point1;
    this->point2 = point2;
}

void Line::draw()
{
	JDL::line(point1.x, point1.y, point2.x, point2.y);
}

//check if the two lines (not segments) intersect
bool Line::intersectsInfinite(const Line &otherLine, Point *resultPoint)
{
    int x1, x2, x3, x4;
    int y1, y2, y3, y4;
    x1 = point1.x;
    x2 = point2.x;
    x3 = otherLine.point1.x;
    x4 = otherLine.point2.x;
    y1 = point1.y;
    y2 = point2.y;
    y3 = otherLine.point1.y;
    y4 = otherLine.point2.y;
    int numX, denomX;
    numX = (x1*y2 - y1*x2)*(x3-x4) - (x1 - x2)*(x3*y4 - y3*x4);
    denomX = (x1 -x2)*(y3 - y4) - (y1 - y2)*(x3 - x4);
    int numY, denomY;
    numY = (x1*y2 - y1*x2)*(y3 - y4) - (y1 - y2)*(x3*y4 - y3*x4);
    denomY = (x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4);
    if (denomX == 0 || denomY == 0)
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
    Point dummyPoint;
    if (!otherLine.pointWithin(Point(intersect.x, intersect.y), CWR, &dummyPoint))
    {
        return false;
    }
    //we now know that this infinite line intersects with the otherLine segment.
    //Now we need to know if this intersection was on the wrong side of the ray.

    //cout << "Points!" << endl;
    //point1.print();
    //point2.print();
    //intersect.print();
    if (point2.x > point1.x)
    { //then decreasing x is bad
        if (intersect.x < point1.x - CWR)
        {
            return false;
        }
    }
    else
    { //increasing x is bad
        if (intersect.x > point1.x + CWR)
        {
            return false;
        }
    }
    
    if (point2.y > point1.y)
    { //decreasing y is bad
        if (intersect.y < point1.y -CWR)
        {
            return false;
        }
    }
    else
    { //increasing y is bad
        if (intersect.y > point1.y +CWR)
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
    Point dummyPoint;
    if (this->pointWithin(Point(temp.x, temp.y), CWR, &dummyPoint) 
        && otherLine.pointWithin(Point(temp.x, temp.y), CWR, &dummyPoint))
    {
        *resultPoint = temp;
        return true;
    }
    return false;
}

bool Line::pointWithin(Point testPoint, int radius, Point *resultPoint) const
{
    double slope;
    int maxY, minY, maxX, minX;
    maxY = max(point2.y, point1.y);
    minY = min(point2.y, point1.y); 
    maxX = max(point2.x, point1.x);
    minX = min(point2.x, point1.x);
    
    if ((testPoint.y <= (maxY + radius))
        && (testPoint.y >= (minY- radius))
        && (testPoint.x <= (maxX + radius)) 
        && (testPoint.x >= (minX - radius)))
    {
        if (point2.x == point1.x)
        { //catch case where line is vertical
            int verticalY;
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
            slope = ((double) point2.y - point1.y) / (point2.x - point1.x);
            int checkY = (int) (slope * testPoint.x + point1.y - (slope * point1.x));
            //cout << "input" << "(" << testPoint.x << "," << testPoint.y << ")" << endl;
            //cout << "test" << " (" << testPoint.x << "," << checkY << ")" << endl;
            if ((testPoint.y >= (checkY - radius))
                && (testPoint.y <= (checkY + radius)))
            {
                int returnX, returnY;
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

bool Line::operator==(const Line &other)
{
    return ((point1 == other.point1) && (point2 == other.point2));
}
