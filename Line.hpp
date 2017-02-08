#ifndef LINE_HPP
#define LINE_HPP

#include <vector>

#include "Crack.hpp"
#include "Shape.hpp"
#include "Point.hpp"

class Shape;
class Crack;

//A Line may have many cracks
//at some point, consider this for efficiency purposes
//lines get passed around a lot. Maybe use those darn &s.
class Line
{
private:
    std::vector<Crack> cracks;

public:
    Point point1;
    Point point2;
    int index;

    Line();
    Line(Point point1, Point point2);

    Line(const Line & other);
    void move(double distance, double degrees);
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
    Line operator=(const Line &other);

    void split(Point splitPoint, Line *newLine);

    void createFracture(Point startPoint, Shape *parentShape, double force);
    void increaseCracks(Point impactPoint, double force);
    
};

#endif
