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
    void draw() const;
    double length() const;

    bool intersectsInfinite(const Line &otherLine, Point *resultPoint);
    bool rayIntersects(const Line &otherLine, Point *resultPoint);
    bool onRay(Point testPoint, double radius, Point *resultPoint);
    bool onRay(Point testPoint);
    bool intersects(const Line &otherLine, Point *resultPoint);
    bool on(Point testPoint, double radius, Point *resultPoint) const;
    bool on(Point testPoint) const;
    bool operator==(const Line &other);
};

#endif
