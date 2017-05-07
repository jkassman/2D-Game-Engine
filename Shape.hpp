#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "Crack.hpp"
#include "Line.hpp"
#include "gfx_j.h"
#include "IEMath.h"

#include <vector>
#include <string>

//#define RESEARCH_SAVE_ALL
//#define RESEARCH_SAVE_ONE
#define RESEARCH_SAVE_STORY
//#define RESEARCH_SAVE_STORY_SHORT

class Crack;

enum BoundType {SHAPE_BOUND_NONE, SHAPE_BOUND_DESTROY, SHAPE_BOUND_BOUNCE,
                SHAPE_BOUND_WRAP};
enum CornerType {SHAPE_CORNER_BOTH, SHAPE_CORNER_NONE, SHAPE_CORNER_LEFT,
                 SHAPE_CORNER_RIGHT, SHAPE_CORNER_UP, SHAPE_CORNER_DOWN};

//A shape is just a collection of LINES
class Shape
{

private:
    std::vector<Line*> lines;
    std::vector<Crack*> cracks;

    double angularVelocity; //in degrees per time step
    double speed; //pixels per time step
    double direction; //in degrees

    BoundType bound;
    int xMinBound;
    int xMaxBound;
    int yMinBound;
    int yMaxBound;
    int ID;
    //int subID;
    int lastHit;
    static int newestID;
    double mass;
    double momentI;
    double estRadius;
    
    //hitbox stuff:
    Point boxTopLeft;
    double width;
    double height;

    bool sanityCheck();
    //void calculateCenter();
    void calculateHitBox();
    void calculateMass();
    void calculateMoment();
    void debugDraw();
    void removeCracksOutside();
    void getCracksOutside(std::vector<Crack*> *toFill);

    void boundBounce();
    CornerType checkCorners();

protected:
    double orientation; //in degrees
    Point center;

public:
    //TODO: Make these private
    std::vector<Shape*> *toDraw; //pointer to a vector of all drawable shapes.
    static int saveNum;
    bool projectile; //TODO: Shouldn't be public
    
    void initNull();
    void init(std::vector<Shape*> *toDraw);
    void updateLines(const std::vector<Line*> &newLines);
    void build(); //creates new center, hitbox, and mass.
    Shape(std::vector<Shape*> *toDraw);
    Shape(std::vector<Point> givenPoints, std::vector<Shape*> *toDraw);
    Shape(std::vector<Line*> &givenLines, std::vector<Shape*> *toDraw);
    Shape(std::string jsonString, std::vector<Shape*> *toDraw);
    Shape(const Shape &other);
    //Whether or not the given point is inside this shape
    bool inside(Point toTest);
    
    void scale(double factor);
  //void setAcc(double acceleration, double direction); //in pixels per timestep
    void accelerate(double acceleration, double degrees);
    void setVelocity(double speed, double degrees); //in pixels per timestep
    void setAngularVelocity(double angVel);
    double getSpeed();
    double getDirection();

    void move();
    void translate(double distance, double degrees);
    void rotate(double theta); //about COM
    void draw();
    void collide(Shape *collider);
    bool collide();
    bool resolveImpulse(Point contactPoint, Shape *B);
    void ApplyImpulse(const Vec2 &impulse, const Vec2 &contactVector);
//^^Modified from Copyright (c) 2013 Randy Gaul http://RandyGaul.net
//(See IEMath.h)
    bool hitBoxOverlapsWith(const Shape &other) const;
    void setBounds(int xMin, int xMax, int yMin, int yMax);
    void copyBounds(const Shape &other);
    void setBoundType(BoundType toSet);
    void checkBounds();
    CornerType cornerOutOfBounds();

    int rayTrace(Line &ray);

    void addPoint(Point toAdd);
    //Line *getLineNearest(const Point clickPoint) const;
    //Crack *addCrack(Point impactPoint);
    //int fractureAt(Point clickPoint);
    int fractureAt(Point clickPoint, double force);
    void distributeForce(Point impactPoint, double force, double radius,
                         std::vector<int> *forces,
                         std::vector<Point> *hitPoints) const;
    Crack * getAffectedCrack(Point impactPoint, double radius);
    bool tryOneSplit();
    void split (const std::vector<Line*> &splitLines, 
                const std::vector<Crack*> &crackLines);
    void saveStory();

    void addCracks(const std::vector<Crack*> &cracksToAdd);
    void deleteCrack(Crack *toDelete);

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
                                    const Crack *toIgnore) const;
    bool lineIntersectsBorderCrackNearest(const Line &toCheck, 
                                          Point *intersectPoint) const;
    int lineIntersectsCrack(const Line &toCheck, 
                             std::vector<Crack*> *intersectCracks,
                            std::vector<Point> *intersectPoints,
                            const Crack *crackToIgnore) const;

    std::string generateJSON() const;
    
    void save(std::string filename);
    
    Shape *getShapeWithCrack(Crack *crackToCheck);

    void splitChildren(std::vector<Crack*> *children);
};

void loadShapes(std::string filename, std::vector<Shape*> *toDraw);
void saveShapes(std::string filename, std::vector<Shape*> *toDraw);
void appendLines(std::vector<Line*> *lines1, std::vector<Line*> &lines2);
void loadLines(std::string fileName, std::vector<Line*> *toFill); //DOESNT WORK
Shape *loadShape(std::string filename, std::vector<Shape*> *toDraw);
void collide1D(double m1, double m2, double &v1i, double &v2i);

#endif
