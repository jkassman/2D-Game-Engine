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
    bool projectile; //TODO: Shouldn't be public

    Shape(std::vector<Shape*> *toDraw);
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
    void collide();

    int rayTrace(Line &ray);

    void addPoint(Point toAdd);
    Line *getLineNearest(const Point clickPoint) const;
    Crack *addCrack(Point impactPoint);
    int fractureAt(Point clickPoint);
    void distributeForce(Point impactPoint, double force, double radius,
                         std::vector<int> *forces,
                         std::vector<Point> *hitPoints) const;
    Crack * getAffectedCrack(Point impactPoint, double radius);
    void split (std::vector<Line*> &splitLines);
    void updateCrackParents();

    void grabShapeLines(Point startPoint, Point endPoint,
                        std::vector<Line*> *result);

    bool lineOnBorder(const Line &toCheck) const;
    bool containsLine(Line *toCheck) const;
    bool pointOn(Point toCheck, double radius, Point *resultPoint) const;
    bool pointOn(Point toCheck) const;
    bool lineIntersects(const Line &toCheck, Point *intersectPoint) const;
    bool lineIntersects(const Line &toCheck) const;
    bool lineIntersectsBorderNearest(const Line &toCheck, 
                                     Point *intersectPoint) const;
    bool lineIntersectsCrackNearest(const Line &toCheck, 
                                    Point *intersectPoint,
                                    Crack **intersectCrack,
                                    Crack *toIgnore) const;
    bool lineIntersectsBorderCrackNearest(const Line &toCheck, 
                                          Point *intersectPoint) const;
    int lineIntersectsCrack(const Line &toCheck, 
                             std::vector<Crack*> *intersectCracks,
                            std::vector<Point> *intersectPoints,
                            Crack *crackToIgnore) const;

    std::string generateJSON();
    
    void save(std::string filename);

};

void splitChildren(std::vector<Crack*> *children);
void appendLines(std::vector<Line*> *lines1, std::vector<Line*> &lines2);
void loadLines(std::string fileName, std::vector<Line*> *toFill);
void parseJsonList(std::string *jsonString, std::vector<std::string> *toFill);
std::string grabJsonObject(std::string *jsonString);
std::string grabJsonValue(std::string jsonString, std::string value);

#endif
