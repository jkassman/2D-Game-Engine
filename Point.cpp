#include "Point.hpp"
#include "JDL.hpp"

#include <iostream>

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

bool Point::operator==(const Point &other)
{
    return (JDL::doublesEqual(x, other.x) && JDL::doublesEqual(y, other.y));
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
