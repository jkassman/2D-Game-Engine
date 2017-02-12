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
    //bool splitting;

    //modifies the current shape and creates a new one.
    //called by increase when the crack intersects a line
    void split(Line *endLine);
    void addPoint(Point toAdd);
    
public:
    Crack(Shape *parentShape, Point startPoint, Line *startLine);
    Crack(const Crack &other);

    //force is a measure of how much the crack should increase in length
    int increase(double force);
    Point startPoint();

    void move(double distance, double degrees);
    void draw() const;
};

#endif
