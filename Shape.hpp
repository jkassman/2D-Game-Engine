#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "Line.hpp"
#include "Crack.hpp"
#include "gfx_j.h"

#include <vector>
#include <string>

//class Crack;
//class Line;

//A shape is just a collection of LINES
class Shape
{
    friend class Crack;

private:
    std::vector<Line*> lines;
    std::vector<Shape*> *toDraw; //pointer to a vector of all drawable shapes.
    double speed;
    double direction; //in degrees

    bool sanityCheck();
    void debugDraw();
    void removeCracksOutside();

    static int saveNum;

public:
    Shape(std::vector<Point> givenPoints, std::vector<Shape*> *toDraw);
    Shape(std::vector<Line*> &givenLines, std::vector<Shape*> *toDraw);
    //Whether or not the given point is inside this shape
    bool inside(Point toTest);
    
    void move();
  //void setAcc(double acceleration, double direction); //in pixels per timestep
    void accelerate(double acceleration, double degrees);
    void setSpeed(double speed, double degrees); //in pixels per timestep
    void move(double distance, double degrees);
    void draw();

    int rayTrace(Line &ray);

    void addPoint(Point toAdd);
    Crack *addCrack(Point impactPoint);
    int fractureAt(Point clickPoint);
    void split (std::vector<Line*> &splitLines);

    void grabShapeLines(Point startPoint, Point endPoint,
                        std::vector<Line*> *result);
/*
    void grabShapeLines(int startIndex, int endIndex,
                        std::vector<Line*> *result);
    void grabShapeLines(std::vector<Line*>::iterator startIndex, 
                        std::vector<Line*>::iterator endIndex, 
                        std::vector<Line*> *result);
*/
    bool lineOnBorder(const Line &toCheck) const;
    int lineIntersectsCrack(const Line &toCheck, 
                             std::vector<Crack*> *intersectCracks,
                            std::vector<Point> *intersectPoints,
                            Crack *crackToIgnore) const;

    std::string generateJSON();
    
    void save(std::string filename);

};

void appendLines(std::vector<Line*> *lines1, std::vector<Line*> &lines2);

#endif
