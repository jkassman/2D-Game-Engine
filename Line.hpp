#ifndef LINE_HPP
#define LINE_HPP

#include "Point.hpp"

class Line
{
public:
    Point point1;
    Point point2;

    Line();
    Line(Point point1, Point point2);
    bool intersects(const Line &otherLine);
    Point pointWithin(Point testPoint, int radius) const;
    bool operator==(const Line &other);
};

#endif
