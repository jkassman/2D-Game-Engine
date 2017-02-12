#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "Line.hpp"
#include "Crack.hpp"
#include "gfx_j.h"

#include <vector>

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

public:
    Shape(std::vector<Point> givenPoints, std::vector<Shape*> *toDraw);
    Shape(std::vector<Line*> &givenLines, std::vector<Shape*> *toDraw);
    //Whether or not the given point is inside this shape
    bool inside(Point toTest);
    
    void move();
  //void setAcc(double acceleration, double direction); //in pixels per timestep
    void setSpeed(double speed, double degrees); //in pixels per timestep
    void move(double distance, double degrees);
    void draw();

    int rayTrace(Line &ray);

    void addPoint(Point toAdd);
    void fractureAt(Point clickPoint);

    void split (Line *startLine, Line *endLine, 
                std::vector<Line*> &splitLines);

//    void split(int indexA1, char posOrNegA, int indexA2, const std::vector<Line*> &CrackA,
    //             int indexB1, char posOrNegB, int indexB2, const std::vector<Line*> &CrackB);
};

#endif
