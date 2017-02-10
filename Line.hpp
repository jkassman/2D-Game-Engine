#ifndef LINE_HPP
#define LINE_HPP

#include <vector>

#include "Shape.hpp"
#include "Crack.hpp"
#include "Point.hpp"

class Shape;
class Crack;

//A Line may have many cracks
class Line
{
private:
    std::vector<Crack*> cracks;

    //void getImpacts(const Point &impactPoint); 

public:
    Point point1;
    Point point2;
    int index;

    Line();
    Line(Point point1, Point point2);

    //Line(const Line & other);
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
    //Line operator=(const Line &other);

    void split(Point splitPoint, Line *newLine);

    //void createFracture(Point startPoint, Shape *parentShape, double force);
    //int numImpacts(Point impactPoint); 

    //eventually, use the force to calculate how many new cracks should
    //be created, if any.
    //or just never create a new crack; keep to some density.
    void increaseCracks(Point impactPoint, Shape *parentShape, double force);
    
};

#endif
