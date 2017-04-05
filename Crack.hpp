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
    //Crack *intersectCrack;
    bool doDelete;
    //Crack *temp;

    Crack *parentCrack;
    Line *startLine;
    bool shapeSplit;

    void addPoint(Point toAdd);
    void init(Shape *parentShape, Point startPoint, Line *startLine);
    
public:
    Shape *parentShape; //TODO: make this private again

    void increaseOne(double force);
    void getChildren(std::vector<Crack*> *children);
    void getGrandestChildren(std::vector<Crack*> *grandChildren);
    void increaseOld(double force);

    Crack* addChild(); //adds a crack that is a point

    Crack(Shape *parentShape, Point startPoint, Line *startLine);
    Crack(Shape *parentShape, const std::vector<Line*> &newLines, Line *start);
    Crack(std::string jsonString, Line *startLine);
    Crack(const Crack &other);

    //force is a measure of how much the crack should increase in length
    void increase(double force);
    Point startPoint();
    bool isPoint();
    Line *getStartLine();
    Line *getFirstLine();

    bool isShapeSplit();
    bool shouldDelete();
    void convertToLinesDelete(std::vector<Line*> *resultVec, Crack *toDelete);
    Crack *splitOffAt(Point splitPoint);
    void getSplitLines(std::vector<Line*> *splits);
    void clearLines();                          
    void setParent(Shape *parentShape);
    Crack *getTopParentCrack();

    void move(double distance, double degrees);
    void draw() const;

    bool lineIntersects(const Line &toCheck, Point *intersect, 
                        Crack **intersectCrack);
    bool lineIntersectsIgnore(const Line &toCheck, Point toIgnore);

    bool sanityCheck(Shape *parentShape, Line *startLine); //NOTE: sanityCheck MODIFIES to make sane!

    std::string generateJSON();
};

#endif
