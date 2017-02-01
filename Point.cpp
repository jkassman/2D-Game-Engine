#include "Point.hpp"

Point::Point()
{
    this->x = 0;
    this->y = 0;
}

Point::Point(int x, int y)
{
    this->x = x;
    this->y = y;
}

Point::Point(const Point &other)
{
    this->x = other.x;
    this->y = other.y;
}

Point Point::operator=(const Point &other)
{
    this->x = other.x;
    this->y = other.y;
    return *this;
}

int Point::operator[](char coord)
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
    return ((x == other.x) && (y == other.y)); 
}
