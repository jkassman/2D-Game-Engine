#ifndef CRACK_HPP
#define CRACK_HPP

#include "Shape.hpp"
#include "Line.hpp"
#include "Point.hpp"

#include <algorithm>
#include <vector>

class Line;
class Shape;

class Crack
{
private:
    std::vector<Line*> lines;
    Line *startLine;
    Shape *parentShape;
    bool point;
    bool shapeSplit;

    void addPoint(Point toAdd);
    
public:
    Crack(Shape *parentShape, Point startPoint, Line *startLine);
    Crack(const Crack &other);

    //force is a measure of how much the crack should increase in length
    void increase(double force);
    Point startPoint();
    Line *getStartLine();

    bool isShapeSplit();
    void getSplitLines(std::vector<Line*> *splits);
    void clearLines();                          
    void setParent(Shape *parentShape);

    void move(double distance, double degrees);
    void draw() const;

    bool lineIntersects(const Line &toCheck);
};

#endif
