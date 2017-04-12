#ifndef LINE_HPP
#define LINE_HPP

#include <vector>
#include <string>

#include "Point.hpp"

//A Line may have many cracks
class Line
{
    //friend class Crack;
private:

    //void getImpacts(const Point &impactPoint); 

public:
    Point point1;
    Point point2;

    //0 for ignore no end points, 1 for point1, 2 for point2, 3 for both.
    int ignorePoint;

    Line();
    Line(const Line &other);
    Line(Point thePoint);
    Line(Point point1, Point point2);
    Line(Point point1, double length, double direction);
    Line(std::string jsonString);

    //Line(const Line & other);
    void scale(double factor);
    void move(double distance, double degrees);
    void draw() const;
    void draw(int index) const;
    double length() const;
    bool isPoint();

    void switchPoints();

    bool intersectsInfinite(const Line &otherLine, Point *resultPoint) const;
    bool rayIntersects(const Line &otherLine) const;
    bool rayIntersects(const Line &otherLine, Point *resultPoint) const;
    bool onRay(Point testPoint, double radius, Point *resultPoint);
    bool onRay(Point testPoint);
    bool intersects(const Line &otherLine, Point *resultPoint) const ;
    bool on(const Point & testPoint, double radius, Point *resultPoint) const;
    bool on(const Point & testPoint) const;
    bool inRect(const Point &testPoint) const;
    bool operator==(const Line &other) const;
    bool coincident(const Line &other) const;
    //Line operator=(const Line &other);

    bool moveAwayFromEdges(double distance, Point *toMove) const;

    void split(Point splitPoint, Line *newLine);

    //int getImpactedCracks(Point clickPoint, Shape *parentShape, 
    //                       std::vector<Crack*> *impactedCracks);
    //void setCrackParents(Shape *crackParentShape);

    double getDirection() const;
    //int deleteCrack(Crack *toDelete);
    //Crack *addCrack(Point impactPoint, Shape *parentShape);

    std::string generateJSON() const;

    //bool sanityCheck(Shape *parentShape);
};

void drawLines(std::vector<Line*> toDraw, double secondsToSleep);

#endif
