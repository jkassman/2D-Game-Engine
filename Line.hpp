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
    friend class Crack;
private:

    //void getImpacts(const Point &impactPoint); 

public:
    std::vector<Crack*> cracks;
    
    Point point1;
    Point point2;
    int index;

    Line();
    Line(const Line &other);
    Line(Point point1, Point point2);
    Line(Point point1, double length, double direction);

    //Line(const Line & other);
    void move(double distance, double degrees);
    void draw() const;
    double length() const;

    void switchPoints();

    bool intersectsInfinite(const Line &otherLine, Point *resultPoint) const;
    bool rayIntersects(const Line &otherLine) const;
    bool rayIntersects(const Line &otherLine, Point *resultPoint) const;
    bool onRay(Point testPoint, double radius, Point *resultPoint);
    bool onRay(Point testPoint);
    bool intersects(const Line &otherLine, Point *resultPoint);
    bool on(const Point & testPoint, double radius, Point *resultPoint) const;
    bool on(const Point & testPoint) const;
    bool operator==(const Line &other) const;
    bool coincident(const Line &other) const;
    //Line operator=(const Line &other);

    void split(Point splitPoint, Line *newLine);

    //void createFracture(Point startPoint, Shape *parentShape, double force);
    //int numImpacts(Point impactPoint); 

    //eventually, use the force to calculate how many new cracks should
    //be created, if any.
    //or just never create a new crack; keep to some density.
    //int increaseCracks(Point impactPoint, Shape *parentShape, double force);

    void getImpactedCracks(Point impactPoint, Shape *parentShape, 
                           double force, std::vector<Crack*> *impactedCracks);
    void setCrackParents(Shape *crackParentShape);

    double getDirection();
    int deleteCrack(Crack *toDelete);

};

void drawLines(std::vector<Line*> toDraw, double secondsToSleep);

#endif
