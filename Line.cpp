#include "Line.hpp"

#include <algorithm>
#include <iostream>

using namespace std;

Line::Line()
{

}

Line::Line(Point point1, Point point2)
{
    this->point1 = point1;
    this->point2 = point2;
}

bool Line::intersects(const Line &otherLine)
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
    int x, y;
    x = numX/denomX;
    y = numY/denomY;
    Point thisResult, otherResult;
    thisResult = this->pointWithin(Point(x, y),1);
    otherResult = otherLine.pointWithin(Point(x, y), 1);
    if ((thisResult.x > -1000000) && (otherResult.x > -1000000))
    {
        return true;
    }
    return false;
}

Point Line::pointWithin(Point testPoint, int radius) const
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
        {
            return Point(point2.x, testPoint.y);
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
                return Point(testPoint.x, checkY);
            }
        }
    }
    return Point(-1000000, -1000000);
}

bool Line::operator==(const Line &other)
{
    return ((point1 == other.point1) && (point2 == other.point2));
}
