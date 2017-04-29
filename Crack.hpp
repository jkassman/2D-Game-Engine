#ifndef CRACK_HPP
#define CRACK_HPP

#include "Shape.hpp"
#include "Line.hpp"
#include "Point.hpp"

#include <algorithm>
#include <vector>
#include <string>

class Shape;

class Crack : public Line
{
private:
    Crack *parentCrack;
    bool shapeSplit;
    std::vector<Crack*> childCracks;

    void setDefaults();
    
public:
    //force is a measure of how much the crack should increase in length
    void expand(double force, Shape *parentShape);
    void getGrandestChildren(std::vector<Crack*> *grandChildren);
    void getAllExceptGrandestChildren(std::vector<Crack*> *parents);

    Crack* addChild(); //adds a crack that is a point

    Crack(Point startPoint);
    Crack(const Line &otherLine);
    Crack(std::string jsonString, std::string jsonLine, Crack *parentCrack);
    Crack(const Crack &other);

    Point startPoint();

    void scale(double factor);
    void translate(double distance, double degrees);
    void rotateAbout(double theta, Point about);
    void draw() const;

    bool isShapeSplit();
    void convertToLinesDelete(std::vector<Line*> *resultVec, Crack *toDelete);
    void convertToLinesAndCracks(std::vector<Line*> *linesVec,
                                 std::vector<Crack*> *cracksVec,
                                 Shape *parentShape);
    Crack *split(Point splitPoint);
    Crack *getParentCrack();
    Crack *getTopParentCrack();

    void getSplitLinesAndCracks(std::vector<Line*> *splitLines, 
                                std::vector<Crack*> *splitCracks,
                                Shape *parentShape);

    bool lineIntersects(const Line &toCheck, Point *intersect, 
                        Crack **intersectCrack);
    bool lineIntersects(const Line &toCheck);

    bool sanityCheck(); 

    std::string generateJSON() const;
};

#endif
