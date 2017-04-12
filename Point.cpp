#include "Point.hpp"
#include "JDL.hpp"

#include "Line.hpp"
#include "jacobJSON.hpp"

#include <iostream>
#include <sstream>
#include <math.h>
#include <cstdlib>

using namespace std;

Point::Point()
{
    this->x = 0;
    this->y = 0;
}

Point::Point(double x, double y)
{
    this->x = x;
    this->y = y;
}

Point::Point(const Point &other)
{
    this->x = other.x;
    this->y = other.y;
}

Point::Point(string jsonString)
{
    string xString, yString;
    xString = grabJsonValue(jsonString, "x");
    yString = grabJsonValue(jsonString, "y");
    
    this->x = JDL::stringToDouble(xString);
    this->y = JDL::stringToDouble(yString);
}

void Point::print() const
{
	cout << "(" << x << "," << y << ")" << endl;
}

Point Point::operator=(const Point &other)
{
    this->x = other.x;
    this->y = other.y;
    return *this;
}

double Point::operator[](char coord)
{
    if ((coord == 0) || (coord == 'x'))
    {
        return x;
    }
    else if ((coord == 1) || (coord == 'y'))
    {
        return y;
    }
    else
    {
        throw 42;
        return 0;
    }
}

bool Point::operator==(const Point &other) const
{
    return (JDL::doublesEqual(x, other.x) && JDL::doublesEqual(y, other.y));
}

bool Point::operator!=(const Point &other)
{
    return !(*this == other);
}

//compare x first, then compare y
bool Point::operator<(const Point &other) const
{
    if (x < other.x)
    {
        return true;
    }
    else if (JDL::doublesEqual(x, other.x))
    {
        if (y < other.y)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool Point::near(const Point &other, double radius)
{
    Line dist(*this, other);
    return (dist.length() < radius);
}

void Point::move(double distance, double degrees)
{
    x += cos(degrees/180*M_PI)*distance;
    y += sin(degrees/180*M_PI)*distance;
}

void Point::drawCircle(int radius) const
{
    JDL::circle(this->x, this->y, radius);
}

string Point::generateJSON() const
{
    string toReturn;
    stringstream streamy;
    streamy << "{" << "\"x\":" << x
            << "," << "\"y\":" << y;
    toReturn = streamy.str();
    toReturn += "}";
    return toReturn;
}
