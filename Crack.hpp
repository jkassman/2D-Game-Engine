#ifndef CRACK_HPP
#define CRACK_HPP

#include "Shape.hpp"
#include "Line.hpp"
#include "Point.hpp"

#include <algorithm>
#include <vector>
#include <string>

class Line;
class Shape;

class Crack
{
private:
    std::vector<Line*> lines;
    Crack *intersectCrack;
    bool doDelete;

    Line *startLine;
    Shape *parentShape;
    bool point;
    bool shapeSplit;

    void addPoint(Point toAdd);
    void init(Shape *parentShape, Point startPoint, Line *startLine);
    
public:
    Crack(Shape *parentShape, Point startPoint, Line *startLine);
    Crack(Shape *parentShape, const std::vector<Line*> &newLines, Line *start);
    Crack(std::string jsonString, Line *startLine);
    Crack(const Crack &other);

    //force is a measure of how much the crack should increase in length
    void increase(double force);
    Point startPoint();
    Line *getStartLine();
    Line *getFirstLine();

    bool isShapeSplit();
    bool shouldDelete();
    void getSplitLines(std::vector<Line*> *splits);
    void clearLines();                          
    void setParent(Shape *parentShape);

    void move(double distance, double degrees);
    void draw() const;

    bool lineIntersects(const Line &toCheck, Point *intersect);

    bool sanityCheck(Shape *parentShape, Line *startLine); //NOTE: sanityCheck MODIFIES to make sane!

    std::string generateJSON();
};

#endif
