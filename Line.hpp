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
	void draw();
    bool intersects(const Line &otherLine, Point *resultPoint);
    bool pointWithin(Point testPoint, int radius, Point *resultPoint) const;
    bool operator==(const Line &other);
};

#endif
