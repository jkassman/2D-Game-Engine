#include "Shape.hpp"
#include "JDL.hpp"
#include "jacobJSON.hpp"

#define _USE_MATH_DEFINES

#include <iostream>
#include <algorithm>
#include <math.h>
#include <fstream>
#include <sstream>
#include <set>

using namespace std;

//corners are 0-3;
int Shape::newestID = 4;

const double Shape::DENSITY_GRAM_PER_METER_CUBED = 2;
const double Shape::PIXELS_TO_METERS = 0.01; //every pixel is one centimeter.
const double Shape::MATERIAL_THICKNESS_2D = 100 / 7 * PIXELS_TO_METERS;
const double Shape::DENSITY_GRAM_PER_METER_SQUARED = DENSITY_GRAM_PER_METER_CUBED / MATERIAL_THICKNESS_2D;
const double Shape::DENSITY_KG_PER_METER_SQUARED = DENSITY_GRAM_PER_METER_SQUARED / 1000.0;

void Shape::initNull()
{
    this->bound = SHAPE_BOUND_NONE;
    this->projectile = false;
    this->speed = 0;
    this->orientation = 0;
    this->angularVelocity = 0;
    this->direction = 0;
    this->xMinBound = 0;
    this->xMaxBound = 400;
    this->yMinBound = 0;
    this->yMaxBound = 400;
   
    this->toDelete = false;

    this->mass = 0;
    this->momentI = 0;

    this->ID = newestID;
    newestID++;
    //this->subID = 0;
    this->center = Point(0,0);
    this->estRadius = 0;
    this->lastHit = -1;
}

void Shape::init(vector<Shape*> *toDraw)
{
    initNull();
    this->toDraw = toDraw;
}

void Shape::build()
{
    calculateHitBox();
    calculateMass();
    calculateMoment();
}

//NOTE: Just copies the pointers into lines vec. does not call constructor.
//does call build, though.
void Shape::updateLines(const vector <Line*> &newLines)
{
    //TODO: Actually delete the old lines. Memory Leaks!
    lines.clear();
    vector<Line*>::const_iterator l;
    for (l = newLines.begin(); l != newLines.end(); ++l)
    {
        lines.push_back(*l);
    }
    if (!sanityCheck())
    {
        debugDraw();
    }
    build();
}

Shape::Shape(vector<Point> givenPoints, vector<Shape*> *toDraw)
{
    init(toDraw);
    vector<Point>::iterator i;
    vector<Point>::iterator next;
#if 0
    cout << "You should not leave your code unattended. hahahaha" << endl;
#endif
    for (i = givenPoints.begin(); i != givenPoints.end(); ++i)
    {
        if ((i + 1) == givenPoints.end())
        {
            next = givenPoints.begin();
        }
        else
        {
            next = i + 1;
        }
        addPoint(*next);
    }
    //calculateCenter();
    build();
}

Shape::Shape(vector<Line*> &givenLines, vector<Shape*> *toDraw)
{
    init(toDraw);
    //this->lines = givenLines;
    vector<Line*>::iterator l;
    for (l = givenLines.begin(); l != givenLines.end(); ++l)
    {
        lines.push_back(*l);
    }
    //calculateCenter();
    build();
}

Shape::Shape(vector<Shape*> *toDraw)
{
    init(toDraw);
}

Shape::Shape(string jsonString, vector<Shape*> *toDraw)
{
    this->toDraw = toDraw;
    string xMinBoundString = grabJsonValue(jsonString, "xMinBound");
    string xMaxBoundString = grabJsonValue(jsonString, "xMaxBound");
    string yMinBoundString = grabJsonValue(jsonString, "yMinBound");
    string yMaxBoundString = grabJsonValue(jsonString, "yMaxBound");
    string boundString = grabJsonValue(jsonString, "bound");
    string speedString = grabJsonValue(jsonString, "speed");
    string directionString = grabJsonValue(jsonString, "direction");
    string angularString = grabJsonValue(jsonString, "angularVelocity");
    string orientationString = grabJsonValue(jsonString, "orientation");
    string linesString = grabJsonValue(jsonString, "lines");
    string cracksString = grabJsonValue(jsonString, "cracks");
    
    this->speed = JDL::stringToDouble(speedString);
    this->direction = JDL::stringToDouble(directionString);
    this->angularVelocity = JDL::stringToDouble(angularString);
    this->orientation = JDL::stringToDouble(orientationString);
    this->xMinBound = JDL::stringToInt(xMinBoundString);
    this->xMaxBound = JDL::stringToInt(xMaxBoundString);
    this->yMinBound = JDL::stringToInt(yMinBoundString);
    this->yMaxBound = JDL::stringToInt(yMaxBoundString);

    vector<string> jsonLines;
    parseJsonList(&linesString, &jsonLines);
    vector<string>::iterator i;
    for (i = jsonLines.begin(); i != jsonLines.end(); ++i)
    {
        lines.push_back(new Line(*i));
    }
    vector<string> jsonCracks;
    parseJsonList(&cracksString, &jsonCracks);
    for (i = jsonCracks.begin(); i != jsonCracks.end(); ++i)
    {
        string jsonLine = grabJsonValue(*i, "line");
        cracks.push_back(new Crack(*i, jsonLine, NULL));
    }

    //determine bound type
    if (boundString == "\"none\"")
    {
        this->bound = SHAPE_BOUND_NONE;
    }
    else if (boundString == "\"destroy\"")
    {
        this->bound = SHAPE_BOUND_DESTROY;
    }
    else if (boundString == "\"bounce\"")
    {
        this->bound = SHAPE_BOUND_BOUNCE;
    }
    else if (boundString == "\"wrap\"")
    {
        this->bound = SHAPE_BOUND_WRAP;
    }
    else
    {
        cerr << "ERROR: Bound type not recognized!" << endl;
    }
    build();
}

Shape::Shape(const Shape &other)
{
    vector<Line*>::const_iterator l;
    vector<Crack*>::const_iterator c;
    for (l = other.lines.begin(); l != other.lines.end(); ++l)
    {
        lines.push_back(new Line(**l));
    }
    for (c = other.cracks.begin(); c != other.cracks.end(); ++c)
    {
        cracks.push_back(new Crack(**c, true, NULL));
    }

    toDelete = other.toDelete;
    angularVelocity = other.angularVelocity;
    speed = other.speed;
    direction = other.direction;
    bound = other.bound;
    xMinBound = other.xMinBound;
    xMaxBound = other.xMaxBound;
    yMinBound = other.yMinBound;
    yMaxBound = other.yMaxBound;
    ID = other.ID;
    lastHit = other.lastHit;
    mass = other.mass;
    momentI = other.momentI;
    estRadius = other.estRadius;

    numPointsInside = other.numPointsInside;
    ignoreFracture = other.ignoreFracture;

    boxTopLeft = other.boxTopLeft;
    width = other.width;
    height = other.height;

    orientation = other.orientation;
    center = other.center;

    toDraw = other.toDraw;
    projectile = other.projectile;
    
}

void Shape::scale(double factor)
{
    vector<Line*>::iterator l;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        (*l)->scale(factor);
    }
    vector<Crack*>::iterator c;
    for (c = cracks.begin(); c != cracks.end(); ++c)
    {
        (*c)->scale(factor);
    }
    center.x *= factor;
    center.y *= factor;
}

 void Shape::translate(double distance, double radians)
 {
     vector<Line*>::iterator l;
     for (l = lines.begin(); l != lines.end(); ++l)
     {
         (*l)->translate(distance, radians);
     }
     vector<Crack*>::iterator c;
     for (c = cracks.begin(); c != cracks.end(); ++c)
     {
         (*c)->translate(distance, radians);
     }
     center.translate(distance, radians);
     boxTopLeft.translate(distance, radians);
 }

void Shape::rotate(double theta)
{
    vector<Line*>::iterator l;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        (*l)->rotateAbout(theta, center);
    }
    vector<Crack*>::iterator c;
    for (c = cracks.begin(); c != cracks.end(); ++c)
    {
        (*c)->rotateAbout(theta, center);
    }
    orientation += theta;
   
}

 void Shape::move(double secondsElapsed)
 {
     translate(speed*secondsElapsed, direction);
     rotate(angularVelocity*secondsElapsed);
 }

 void Shape::setVelocity(double speed, double radians)
 {
     this->speed = speed;
     this->direction = radians;
 }

void Shape::setAngularVelocity(double angVel)
{
    angularVelocity = angVel;
}

double Shape::getSpeed()
{
    return speed;
}

double Shape::getDirection()
{
    return direction;
}

double Shape::getMass()
{
    return mass;
}

 void Shape::draw()
 {
     JDL::point(center.x, center.y);
     vector<Line*>::iterator l;
     for (l = lines.begin(); l != lines.end(); ++l)
     {
         (*l)->draw();
     }
     vector<Crack*>::iterator c;
     for (c = cracks.begin(); c != cracks.end(); ++c)
     {
         (*c)->draw();
     }
 }

void Shape::deleteSelfFromToDraw()
{
    vector<Shape*>::iterator s;
    for (s = toDraw->begin(); s != toDraw->end(); ++s)
    {
        if (*s == this)
        {
            toDraw->erase(s);
        }
    }
}

bool Shape::hitBoxOverlapsWith(const Shape &other) const
{
    if (boxTopLeft.x < (other.boxTopLeft.x + other.width))
    {
        if (boxTopLeft.x + width > other.boxTopLeft.x)
        {
            if (boxTopLeft.y < (other.boxTopLeft.y + other.height))
            {
                if ((boxTopLeft.y + height) > other.boxTopLeft.y)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void Shape::collide(Shape *b, Point cornerPoint)
{
    Shape *a = this;
    Point bPoint;
    if (!b->pointOn(cornerPoint, 30, &bPoint))
    {
        cerr << "The impact point was not near enough Shape b!" << endl;
        return;
    }

    //Main idea: calculate impulse then use that to calculate other stuff.
    //Main step 1: calculate Impulse
    //Substep 1: calculate vab1
    Line va1Line(Point(0, 0), a->speed *PIXELS_TO_METERS, a->direction);
    Line vb1Line(Point(0, 0), b->speed *PIXELS_TO_METERS, b->direction);

    Vec2 va1(va1Line.point2.x, va1Line.point2.y);
    Vec2 vb1(vb1Line.point2.x, vb1Line.point2.y);
    Vec2 rap((cornerPoint.x - a->center.x) * PIXELS_TO_METERS,
             (cornerPoint.y - a->center.y) * PIXELS_TO_METERS);
    Vec2 rbp((bPoint.x - b->center.x) * PIXELS_TO_METERS, 
             (bPoint.y - b->center.y) * PIXELS_TO_METERS);
    

    Vec2 vap1 = va1 + Cross(a->angularVelocity, rap);
    Vec2 vbp1 = vb1 + Cross(b->angularVelocity, rbp);

    Vec2 vab1 = vap1 - vbp1;

    //substep 2: calculate n
    Line normalLine = b->getNormalLine(bPoint);

    Vec2 n(normalLine.point2.x - normalLine.point1.x,
           normalLine.point2.y - normalLine.point1.y);
    n.Normalize();

    //substep 3: define elasticity
    double elasticity = 1;

    //substep 4: Actually calculate impulse
    double numerator = -(1 + elasticity) * Dot(vab1, n);
    double denominator = 1.0/a->mass
                       + 1.0/b->mass
                       + (Cross(rap, n) * Cross(rap, n))/a->momentI
                       + (Cross(rbp, n) * Cross(rbp, n))/b->momentI;
    double impulse = numerator / denominator;

    //main step 2: get va2 and vb2 (final a and b linear velocities)
    Vec2 va2 = va1 + (impulse * n) / a->mass;
    Vec2 vb2 = vb1 - (impulse * n) / b->mass;

    //Main step 3: get angular velocities
    double wa2 = a->angularVelocity + Cross(rap, impulse * n) / a->momentI;
    double wb2 = b->angularVelocity - Cross(rbp, impulse * n) / b->momentI;

    //Main step 4: change the shapes to reflect the collision
    Line va2Line(Point(0, 0), Point(va2.x, va2.y));
    Line vb2Line(Point(0, 0), Point(vb2.x, vb2.y));
    a->speed = va2Line.length()/PIXELS_TO_METERS;
    a->direction = va2Line.getDirection();

    b->speed = vb2Line.length()/PIXELS_TO_METERS;
    b->direction = vb2Line.getDirection();

    a->angularVelocity = wa2;
    b->angularVelocity = wb2;

    //MAIN THING TODO: UNITS!!!! (speed, mostly)
}           

bool Shape::collide()
{
    if (ignoreFracture > 0) ignoreFracture--;
    vector<Shape*>::iterator s;
    vector<Line*>::iterator l;
    bool toReturn = false;
    for (s = toDraw->begin(); s != toDraw->end(); ++s)
    {
        if ((*s) == this) continue;
        if ((lastHit == (*s)->ID) && ((*s)->lastHit == ID)) continue;
        //if ((*s)->ID == ID) continue;
        if (this->hitBoxOverlapsWith(**s))
        {
            for (l = this->lines.begin(); l != this->lines.end(); ++l)
            {
                Point cornerPoint = (*l)->point1;
                if ((*s)->inside(cornerPoint))
                {
                    //collide(*s);
                    collide(*s, cornerPoint);
                    this->lastHit = (*s)->ID;
                    (*s)->lastHit = this->ID;
                    if (ignoreFracture < 1)
                    {
                        this->fractureAt(cornerPoint, 25);
                        (*s)->fractureAt(cornerPoint, 25);
                        this->ignoreFracture = 20;
                        (*s)->ignoreFracture = 20;
                    }
                    toReturn = true;
                    return true;
                }
            }
        }
    }
    return toReturn;
}

void Shape::setBounds(int xMin, int xMax, int yMin, int yMax)
{
    xMinBound = xMin;
    xMaxBound = xMax;
    yMinBound = yMin;
    yMaxBound = yMax;
}

void Shape::setBoundType(BoundType toSet)
{
    this->bound = toSet;
}

void Shape::copyBounds(const Shape &other)
{
    this->xMinBound = other.xMinBound;
    this->xMaxBound = other.xMaxBound;
    this->yMinBound = other.yMinBound;
    this->yMaxBound = other.yMaxBound;
    this->bound = other.bound;
}

void Shape::checkBounds()
{
    switch (this->bound)
    {
    case SHAPE_BOUND_NONE:
        break;
    case SHAPE_BOUND_DESTROY:
    case SHAPE_BOUND_WRAP:
        cerr << "Bounding strategy not yet implemented!" << endl;
        break;
    case SHAPE_BOUND_BOUNCE:
        boundBounce();
        break;
    }
}

void Shape::boundBounce()
{
    CornerType corner;
    corner = cornerOutOfBounds();
    double xMagnitude = cos(direction) * speed;
    double yMagnitude = sin(direction) * speed;
    switch (corner)
    {
    case SHAPE_CORNER_LEFT:
        //make the shape go right
        if (xMagnitude < 0) xMagnitude = -xMagnitude;
        //else cout << "Already going right, skipping" << endl;
        lastHit = -1;
        break;
    case SHAPE_CORNER_RIGHT:
        //make the shape go left;
        if (xMagnitude > 0) xMagnitude = -xMagnitude;
        //else cout << "Already going left, skipping" << endl;
        lastHit = -2;
        break;
    case SHAPE_CORNER_UP:
        //make the shape go down;
        if (yMagnitude < 0) yMagnitude = -yMagnitude;
        //else cout << "Already going down, skipping" << endl;
        lastHit = -3;
        break;
    case SHAPE_CORNER_DOWN:
        //make the shape go up;
        if (yMagnitude > 0) yMagnitude = -yMagnitude;
        //else cout << "Already going up, skipping" << endl;
        lastHit = -4;
        break;
    case SHAPE_CORNER_BOTH:
        this->speed = -this->speed;
        cout << "Double bounce magic!" << endl;
        lastHit = -5;
        return;
    case SHAPE_CORNER_NONE:
        return;
    }
    Line newVelocity(Point(0,0), Point(xMagnitude, yMagnitude));
    this->direction = newVelocity.getDirection();
    this->speed = newVelocity.length();
}

//TODO: Actually check for SHAPE_CORNER_BOTH
CornerType Shape::cornerOutOfBounds()
{
    vector<Line*>::iterator l;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        double x = (*l)->point2.x;
        double y = (*l)->point2.y;
        if (x < xMinBound)
        {
            return SHAPE_CORNER_LEFT;
        }
        if (x > xMaxBound)
        {
            return SHAPE_CORNER_RIGHT;
        }
        if (y < yMinBound)
        {
            return SHAPE_CORNER_UP;
        }
        if (y > yMaxBound)
        {
            return SHAPE_CORNER_DOWN;
        }
    }
    return SHAPE_CORNER_NONE;
}

//new idea!
//get all cracks in a given (simple) radius
//(if none, create)
//increase the stuff and things?
//increase the grandest children once, and everyone else randomly.

//NOTE: decent idea, but never finished.
#if 0
int Shape::fractureAt(Point clickPoint, double force)
{
    double radius = 10;
    Point impactPoint;
    if (!pointOn(clickPoint, radius, &impactPoint))
    {
        return 0;
    }
    vector<Crack*> affectedCracks;
    /* //TODO: Make able to grab multiple cracks
    vector<Crack*>::iterator c;
    for (c = cracks.begin(); c != cracks.end(); ++c)
    {
        if (c.point1.near(impactPoint, 10))
        {
            affectedCracks.push_back(*c);
        }
    }
    */
    Crack *toIncrease;
    toIncrease = getAffectedCrack(impactPoint, radius);
    vector<Crack*> grandestChildren;
    toIncrease->getGrandestChildren(&grandestChildren);
    vector<Crack*>::iterator c;
    for (c = grandestChildren.begin(); c != grandestChildren.end(); ++c)
    {
        if (!(*c)->isShapeSplit())
        {
            if ((*c)->isPoint())
            {
                (*c)->expand(force, this);
            }
            else
            {
                Crack *newChild = (*c)->addChild();
                newChild->expand(force, this);
            }
        }

    }

    vector<Crack*> parents;
    int numParentsExpanded = 0;
    toIncrease->getAllExceptGrandestChildren(&parents);
    for (c = parents.begin(); c != parents.end(); ++c)
    {
        if ((*c)->isShapeSplit())
        {
            cerr << "A crack with children has split the shape."
                 << " This shouldn't be possible" << endl;
        }
        if (JDL::percentChance(50))
        {
            Crack *newChild = (*c)->addChild();
            newChild->expand(force, this);
            numParentsExpanded++;
        }

    }
    return grandestChildren.size() + numParentsExpanded;
}

#else
 //returns how many cracks were affected
int Shape::fractureAt(Point clickPoint, double forceIgnored)
 {
     vector<Crack*> children;
     vector<Line*> splitLines;
     vector<Crack*> splitCracks;
     Point impactPoint;

     double force = 25;
     double radius = 10;

     Crack *toIncrease = NULL;

     if (!pointOn(clickPoint, radius, &impactPoint))
     {
         return 0;
     }
     toIncrease = getAffectedCrack(impactPoint, radius);

     toIncrease->getGrandestChildren(&children);
     splitChildren(&children); //TODO: Is this the best place for this?    
     vector<Crack*>::iterator c;
     for (c = children.begin(); c != children.end(); ++c)
     {
         (*c)->expand(force, this);
     }

     return children.size();
}
#endif

//if there are any cracks with isShapeSplit == true,
//run split on the first one found.
//Return true if any split occured, false otherwise.
bool Shape::tryOneSplit()
{
    vector<Line*> splitLines;
    vector<Crack*> splitCracks;
    vector<Crack*> grandChildren;
    
    vector<Crack*>::iterator c, c2;
    for (c = cracks.begin(); c != cracks.end(); ++c)
    {
        grandChildren.clear();
        (*c)->getGrandestChildren(&grandChildren);
        for (c2 = grandChildren.begin(); c2 != grandChildren.end(); ++c2)
        {
            if ((*c2)->isShapeSplit())
            {
                (*c2)->getSplitLinesAndCracks(&splitLines, &splitCracks, this);
                split(splitLines, splitCracks);
                return true;
            }
        }
    }
    return false;
}

void Shape::addPoint(Point toAdd)
{
    if (!lines.size())
    {
        lines.push_back(new Line(toAdd, toAdd));
    }
    else
    {
        lines.back()->point2 = toAdd;
        lines.push_back(new Line(toAdd, lines[0]->point1));
    }
    //calculateCenter();
    build();

    /*
    vector<Line*>::iterator l;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        (*l)->point1.print();
    }*/
}
 
//returns the nearest crack within radius.
//If there is no crack within radius, create a new crack at impactPoint
Crack *Shape::getAffectedCrack(Point impactPoint, double radius)
{
    vector<Crack*>::iterator c;
    Crack * closestCrack = NULL;
    double minDist = radius;
    for (c = cracks.begin(); c != cracks.end(); ++c)
    {
        double dist = Line(impactPoint, (*c)->startPoint()).length();
        if (dist < minDist)
        {
            minDist = dist;
            closestCrack = *c;
        }
    }
    if (closestCrack == NULL)
    {
        closestCrack = new Crack(impactPoint);
        cracks.push_back(closestCrack);
    }
    return closestCrack;
}

bool Shape::containsLine(Line *toCheck) const
{
    vector<Line*>::const_iterator l;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l) == toCheck)
        {
            return true;
        }
    }
    return false;
}

//returns the closest point
bool Shape::pointOn(Point toCheck, double radius, Point *resultPoint) const
{
    vector<Line*>::const_iterator l;
    Point minPoint;
    double minDist;
    bool foundPoint = false;
    minDist = Line(toCheck, lines[0]->point1).length();
    minPoint = lines[0]->point1;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l)->on(toCheck, radius, resultPoint))
        {
            foundPoint = true;
            double dist = Line(toCheck, *resultPoint).length();
            if (dist < minDist)
            {
                minPoint = *resultPoint;
                minDist = dist;
            }
        }
    }
    *resultPoint = minPoint;
    return foundPoint;
}
bool Shape::pointOn(Point toCheck) const
{
    Point dummyPoint;
    return pointOn(toCheck, JDL::PRECISION, &dummyPoint);
}

bool Shape::lineIntersects(const Line &toCheck, Point *intersectPoint) const
{
    vector<Line*>::const_iterator l;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l)->intersects(toCheck, intersectPoint))
        {
            return true;
        }
    }
    return false;
}

bool Shape::lineIntersectsBorderNearest(const Line &toCheck, 
                                  Point *intersectPoint) const
{
    vector<Line*>::const_iterator l;
    vector<Point> intersectPoints;
    int numIntersects = 0;

    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l)->intersects(toCheck, intersectPoint))
        {
            numIntersects++;
            intersectPoints.push_back(*intersectPoint);
        }
    }

    vector<Point>::const_iterator p;
    if (intersectPoints.size() == 0) return false;

    double minDist = Line(toCheck.point1, *intersectPoints.begin()).length();
    *intersectPoint = *intersectPoints.begin();
    for (p = intersectPoints.begin()+1; p != intersectPoints.end(); ++p)
    {
        Line dist(toCheck.point1, *p);
        if (dist.length() < minDist)
        {
            minDist = dist.length();
            *intersectPoint = *p;
        }
    }
    return true;    
}

bool Shape::lineIntersectsCrackNearest(const Line &toCheck, 
                                       Point *intersectPoint,
                                       Crack **intersectCrack,
                                       const Crack *toIgnore) const
{
    vector<Crack*> intersectCracks;
    vector<Point> intersectPoints;
    lineIntersectsCrack(toCheck, &intersectCracks, &intersectPoints,
                        toIgnore);

    vector<Point>::iterator p;
    vector<Crack*>::iterator c = intersectCracks.begin();

    if (intersectPoints.size() == 0) return false;
    
    double minDist = Line(toCheck.point1, *intersectPoints.begin()).length();
    *intersectPoint = *intersectPoints.begin();
    *intersectCrack = *intersectCracks.begin();
    c = intersectCracks.begin()+1;
    for (p = intersectPoints.begin()+1; p != intersectPoints.end(); ++p)
    {
        Line dist(toCheck.point1, *p);
        if (dist.length() < minDist)
        {
            minDist = dist.length();
            *intersectPoint = *p;
            *intersectCrack = *c;
        }
        c++;
    }
    
    return true;
}

//returns the number of intersect points found
//ignores crackToIgnore
int Shape::lineIntersectsCrack(const Line &toCheck, 
                               vector<Crack*> *intersectCracks, 
                               vector<Point>  *intersectPoints,
                               const Crack *crackToIgnore) const
{
    vector<Crack*>::const_iterator c;
    Crack *intersectCrack = NULL;
    int numIntersects = 0;
    Point intersect;
    for (c = cracks.begin(); c != cracks.end(); ++c)
    {
        if ((*c) == crackToIgnore)
        {
            continue;
        }
        if ((*c)->lineIntersects(toCheck, &intersect, &intersectCrack))
        {
            intersectCracks->push_back(intersectCrack);
            intersectPoints->push_back(intersect);
            
            numIntersects++;
        }
    }
    
    return numIntersects;
}

bool Shape::lineIntersects(const Line &toCheck) const
{
    Point dummyPoint;
    return lineIntersects(toCheck, &dummyPoint);
}

bool Shape::lineIntersectsBorderCrackNearest(const Line &toCheck,
                                             Point *intersectPoint) const
{
    Point shapeIntersectPoint, crackIntersectPoint;
    bool intersectsBorder, intersectsCrack;
    Crack *dummy;
    intersectsBorder=lineIntersectsBorderNearest(toCheck, &shapeIntersectPoint);
    intersectsCrack = lineIntersectsCrackNearest(toCheck, &crackIntersectPoint, 
                                                 &dummy, NULL);

    bool toReturn = true;

    if ((intersectsBorder) && (intersectsCrack))
    {
        Line borderDist(toCheck.point1, shapeIntersectPoint);
        Line crackDist(toCheck.point1, shapeIntersectPoint);
        if (borderDist.length() > crackDist.length())
        {
            *intersectPoint = shapeIntersectPoint;
        }
        else
        {
            *intersectPoint = crackIntersectPoint;
        }
    }
    else if (intersectsBorder)
    {
        *intersectPoint = shapeIntersectPoint;
    }
    else if (intersectsCrack)
    {
        *intersectPoint = crackIntersectPoint;
    }
    else
    {
        toReturn = false;
    }
    return toReturn;
}

Line Shape::getNormalLine(Point onShape)
{
    vector<Line*>::iterator l;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l)->on(onShape))
        {
            double testDirection = (*l)->getDirection() + (M_PI / 2);
            Line testNormal(onShape, .1, testDirection);
            if (!inside(testNormal.point2))
            {
                return testNormal;
            }
            else
            {
                return (Line(onShape, .1, (*l)->getDirection() - M_PI / 2));
            }
        }
    }
    cerr << "The point for a normal line does not exist on the shape!" << endl;
    return Line(Point(0, 0), Point(0, 0));
}

bool Shape::lineOnBorder(const Line &toCheck) const
{
    vector<Line*>::const_iterator l;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l)->coincident(toCheck))
        {
            return true;
        }
    }
    return false;
}

//returns the number of lines in the shape that are intersected.
//Assumes the line is a ray starting at point1 and going to point2.
int Shape::rayTrace(Line &ray)
{
    //check to see how many lines in the shape the ray intersects with
    Point intersectPoint;
    vector<Line*>::iterator i;
    int numIntersects = 0;
    //vector<Point> intersectPoints;
    set<Point> intersectPoints;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        if (ray.rayIntersects(**i, &intersectPoint))
        {
            //intersectPoints.push_back(intersectPoint);
            intersectPoints.insert(intersectPoint);
            numIntersects++;
        }
    }
    return intersectPoints.size();
/*
    //check for duplicates:
    if (numIntersects != 0)
    {
        sort(intersectPoints.begin(), intersectPoints.end());
        vector<Point>::iterator j;
        Point previous = *intersectPoints.begin();
        for (j = intersectPoints.begin()+1; j != intersectPoints.end(); ++j)
        {
            if (previous == *j)
            {
                cout << "Found a duplicate!" << endl;
                //numIntersects--;
            }
            previous = *j;
        }
    }
    return numIntersects;
*/
}

//split the shape with the given splitLines
//the result should be two new shapes added to toDraw, and this shape deleted.
void Shape::split(const vector<Line*> &splitLines, 
                  const vector<Crack*> &splitCracks)
{
    //endLine splits intersectPoint->point2.
    vector<Line*> lines1, lines2;
    vector<Line*>::const_iterator l;
    vector<Crack*>::iterator c;
    
    Point crackStartPoint, crackEndPoint;
    crackStartPoint = splitLines[0]->point1;
    crackEndPoint = splitLines.back()->point2;
    if (crackStartPoint == crackEndPoint)
    {
        cerr << "The start point and end point of the crack cannot be equal"
             << endl;
        return;
    }
//#define ND_RESEARCH_DEBUG
#ifdef ND_RESEARCH_DEBUG
    crackStartPoint.drawCircle(3);
    crackEndPoint.drawCircle(3);
    JDL::flush();
    JDL::sleep(1);
#endif
    grabShapeLines(crackStartPoint, crackEndPoint, &lines1);
    grabShapeLines(crackEndPoint, crackStartPoint, &lines2);
#ifdef ND_RESEARCH_DEBUG
    JDL::setDrawColor(0, 0, 255);
    drawLines(lines1, 0);
    JDL::setDrawColor(255, 255, 0);
    drawLines(lines2, 0);
    //JDL::sleep(5);
    JDL::setDrawColor(255, 255, 255);
#endif

    /*
      Finished grabbing the relevant lines from the shape.
      Time to copy the splitlines and and add them to the line vectors:
    */

    //copy splitLines
    vector<Line*> splitLinesCopy1;
    vector<Line*> splitLinesCopy2;
    for (l = splitLines.begin(); l != splitLines.end(); ++l)
    {
        splitLinesCopy1.push_back(new Line(**l));
        splitLinesCopy2.push_back(new Line(**l));
    }
    appendLines(&lines1, splitLinesCopy1);
    appendLines(&lines2, splitLinesCopy2);
    
    /*
      Finished creating the new lines for the shapes.
      Time to make shapes from these lines:
    */
    /* Modify this shape (using lines2) */
    //reassign the lines in this shape to be those of lines2.

    this->addCracks(splitCracks);
    Shape *newShape = new Shape(*this);
    newestID++;
    newShape->ID = newestID;
    newestID++;

    this->updateLines(lines2);
    newShape->updateLines(lines1);

    this->removeCracksOutside();
    newShape->removeCracksOutside();

    toDraw->push_back(newShape);

    //make the new shape move away from the current shape.
    double newDirection = 0;    
    Line crackDirection(splitLines[0]->point1, splitLines.back()->point2);
    
    int modifier = 1;
    //if (sameLineSplit) modifier = -1;
    newDirection = crackDirection.getDirection() + (modifier*M_PI/2);
    Line newDirectionRay(splitLines[0]->point1, newDirection, 42);
    if (rayTrace(newDirectionRay) % 2)
    {
        //if odd, so in the direction of the shape
        newDirection -=M_PI;
    }
    newShape->accelerate(40, newDirection);
    newShape->translate(3, newDirection);

    //TODO: Delete splitlines here! We made two copies
    
#ifdef RESEARCH_SAVE_ONE
    save("saves/After_Split.txt");
#endif
#ifdef RESEARCH_SAVE_STORY
    cout << "SAVING" << saveNum << endl;
    stringstream streamy;
    streamy << "story/save" << saveNum << ".txt";  
    saveShapes(streamy.str(), toDraw);
    saveNum++;
#endif

}

void Shape::saveStory()
{
    stringstream streamy;
    streamy << "story/save" << saveNum << ".txt";
    saveShapes(streamy.str(), toDraw);
    saveNum++;
}

void Shape::addCracks(const std::vector<Crack*> &cracksToAdd)
{
    vector<Crack*>::const_iterator c;
    for (c = cracksToAdd.begin(); c != cracksToAdd.end(); ++c)
    {
        cracks.push_back(*c);
    }
}

void Shape::deleteCrack(Crack *toDelete)
{
    vector<Crack*>::iterator c;
    for (c = cracks.begin(); c != cracks.end(); ++c)
    {
        if (toDelete == *c)
        {
            cracks.erase(c);
            return;
        }
    }
    cerr << "ERROR: Failed to delete Crack!" << endl;
}

//grabs all lines in between startPoint and endPoint
//splits them if necessary. (NOT const)
void Shape::grabShapeLines(Point startPoint, Point endPoint,
                           vector<Line*> *result)
{
    /*
      step 1: find the first line that contains startPoint.
      either split it or not.
      -if split, edit shape to have both pieces.
      At the end of the loop, l is the first line to grab.
    */
    vector<Line*>::iterator l;
    //should break before reaching lines.end().
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l)->on(startPoint))
        {
            if (startPoint == (*l)->point1)
            {
                //do nothing
            }
            else if (startPoint == (*l)->point2)
            {
                //no split required, nothing to push back.
                ++l;
            }
            else
            {
                Line *newLine = new Line();
                (*l)->split(startPoint, newLine);
                l = lines.insert(l+1, newLine);
            }
            break;
        }
    }
    if (l == lines.end())
    {
        cerr << "The crack start point is not on the shape!" << endl;
        return;
    }
    
    /*
      Step 2:
      push back everything until we hit the endPoint.
      then either split or not.
      If split, edit shape to have both pieces
    */
    
    //note: we may have to wrap around the shape here
    //the only exit is via breaks. yay standards.
    while (true)
    {
        if ((*l)->on(endPoint))
        {
            if (endPoint == (*l)->point1)
            {
                //no split required, don't push anything back.
            }
            else if (endPoint == (*l)->point2)
            {
                result->push_back(*l);
            }
            else
            {
                Line *newLine = new Line();
                (*l)->split(endPoint, newLine);
                result->push_back(*l);
                lines.insert(l+1, newLine);
            }
            break;
        }
        result->push_back(*l);
        ++l;
        if (l == lines.end())
        {
            l = lines.begin();
        }
        if (result->size() > lines.size())
        {
            result->clear();
            cerr << "The crack end point is not on the shape!" << endl;
            return;
        }
    }
}

void appendLines(std::vector<Line*> *lines1, std::vector<Line*> &lines2)
{    

    Line *lastLine1 = lines1->back();

    Point lastPoint2 = lines2.back()->point2;
    Point firstPoint2 = lines2[0]->point1;
    
    //if the first point of the first line in lines2 
    //is on the last line of lines1
    if (lastLine1->point2==firstPoint2)
    {
        //standard combine
        vector<Line*>::iterator i;
        for (i = lines2.begin(); i != lines2.end(); ++i)
        {
            lines1->push_back(*i);
        }
    }
    //if the last point of the last line in lines2
    //is on the last line of lines1
    else if (lastLine1->point2 ==lastPoint2)
    {
        //reverse combine
        vector<Line*>::reverse_iterator i;
        for (i = lines2.rbegin(); i != lines2.rend(); ++i)
        {
            //switch point1 and point2.
            (*i)->switchPoints();
            lines1->push_back(*i);
        }
    }
    else
    {
        cerr << "The lines groups cannot be merged!" << endl;
        char garbage;
        vector<Line*>::iterator debug;
        JDL::clear();
        for (debug = lines1->begin(); debug != lines1->end(); ++debug)
        {
            cin >> garbage;
            (*debug)->draw(1);
            JDL::flush();
        }

     
        JDL::circle(firstPoint2.x, firstPoint2.y, 5);
        JDL::circle(lastPoint2.x, lastPoint2.y, 5);


        for (debug = lines2.begin(); debug != lines2.end(); ++debug)
        {
            cin >> garbage;
            //int tempIndex = (*debug)->index;
            //(*debug)->index = 2;
            (*debug)->draw(2);
            //(*debug)->index = tempIndex;
            JDL::flush();
        }
    }
}

void Shape::accelerate(double acceleration, double radians)
{
    Point origin(0,0);
    Line velocity(origin, this->speed, this->direction);
    velocity.point2.x += acceleration * cos(radians);
    velocity.point2.y += acceleration * sin(radians);
    
    this->direction = velocity.getDirection();
    this->speed = velocity.length();
}

void Shape::calculateHitBox()
{
    if (lines.size() == 0) 
    {
        return;
    }
    //discover leftmost corner:
    vector<Line*>::iterator l;
    double leftmostX = lines.front()->point1.x;
    double rightmostX = lines.front()->point1.x;
    double downmostY = lines.front()->point1.y;
    double upmostY = lines.front()->point1.y;
    for (l = lines.begin()+1; l != lines.end(); ++l)
    {
        if ((*l)->point1.x < leftmostX)
        {
            leftmostX = (*l)->point1.x;
        }
        if ((*l)->point1.x > rightmostX)
        {
            rightmostX = (*l)->point1.x;
        }
        if ((*l)->point1.y > downmostY)
        {
            downmostY = (*l)->point1.y;
        }
        if ((*l)->point1.y < upmostY)
        {
            upmostY =  (*l)->point1.y;
        }
    }
    boxTopLeft = Point(leftmostX, upmostY);
    width = rightmostX - leftmostX;
    height = downmostY - upmostY;
}

//#define MASS_DEBUG
//assumes that the hitbox is set up already.
void Shape::calculateMass()
{
    #ifdef MASS_DEBUG
        JDL::clear();
        JDL::setDrawColor(255, 0, 255);
    #endif
    int totalX = 0, totalY = 0;
    int w = 0, h = 0;
    mass = 0;
    int skipFactor = 2;
    numPointsInside = 0;
    for (w = boxTopLeft.x; w < boxTopLeft.x + width; w += skipFactor)
    {
        for (h = boxTopLeft.y; h < boxTopLeft.y + height; h += skipFactor)
        {
            if (inside(Point(w, h)))
            {
                //cout << w << "," << h << endl;
                #ifdef MASS_DEBUG
                    Line temp(Point(w, h), Point(w, h));
                    temp.draw();
                #endif
                totalX += w;
                totalY += h;
                mass += (skipFactor*PIXELS_TO_METERS) * (skipFactor*PIXELS_TO_METERS);
                numPointsInside++;
            }
        }
    }
    if (mass <= 30*PIXELS_TO_METERS*PIXELS_TO_METERS)
    {
        this->toDelete = true;
    }
    else
    {
        this->toDelete = false;
    }
    center = Point((double)totalX/numPointsInside, (double)totalY/numPointsInside);

#ifdef MASS_DEBUG
    JDL::setDrawColor(255, 255, 255);
    this->draw();
    JDL::flush();
    char garbage;
    cin >> garbage;
    //JDL::sleep(5);
#endif
}

//assumes mass and hitbox have been set up already
void Shape::calculateMoment()
{
    double w, h;
    int skipFactor = 2;
    //double totalDistSquared = 0;
    momentI = 0;
    for (w = boxTopLeft.x; w < boxTopLeft.x + width; w += skipFactor)
    {
        for (h = boxTopLeft.y; h < boxTopLeft.y + height; h += skipFactor)
        {
            if (inside(Point(w, h)))
            {
                double xDist = (w - center.x) * PIXELS_TO_METERS;
                double yDist = (h - center.y) * PIXELS_TO_METERS;
                double distSquared = xDist*xDist + yDist*yDist;
                momentI += distSquared * mass / numPointsInside;
            }
        }
    }
}


//return true if the shape is closed, return false otherwise.
//TODO: this is a quick sanity check. Figure 8s won't be checked.
bool Shape::sanityCheck()
{
    vector<Line*>::iterator l;
    /*
    vector<Crack*>::iterator c;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        (*l)->sanityCheck(this);
    }
    */
    if (lines.size() == 0) return true; //empty shapes are fine
    if (lines.size() == 1) //This should be a point
    {
        if (lines[0]->point1 == lines[0]->point2)
        {
            return true;
        }
        return false;
    }
    //at this point, lines has 2+ lines
    Line *previous = lines[0];
    for (l = lines.begin() + 1; l != lines.end(); ++l)
    {
        if ((*l)->point1 != previous->point2)
        {
            cout << "Found a discrepency!" << endl;
            JDL::circle((*l)->point1.x, (*l)->point1.y, 3);
            JDL::circle(previous->point2.x, previous->point2.y, 3);
            JDL::flush();
            (*l)->point1.print();
            previous->point2.print();
            return false;
        }
        previous = *l;
    }
    
    //check that the last line connects with the first
    if (lines.back()->point2 != lines[0]->point1)
    {
        cout << "The last line does not connect with the first" << endl;
        JDL::circle((*l)->point1.x, (*l)->point1.y, 3);
        JDL::circle(previous->point2.x, previous->point2.y, 3);
        JDL::flush();
        lines.back()->point2.print();
        lines[0]->point1.print();
        return false;
    }

    return true;
}

//draws the shape, one line at a time, in order, then asks to redraw.
void Shape::debugDraw()
{
    cout << "Found an error in a shape! Turn on debugging to explore it." << endl;
    return;
    char selector;
    cout << "begin the debug draw? (yes:any/no: n)" << endl;
    cin >> selector;
    if (selector == 'n')
    {
        return;
    }
    vector<Line*>::iterator l;
    JDL::clear();
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        (*l)->draw();
        cout << "Drew line " << l - lines.begin() << "." << endl;
        JDL::flush();
        JDL::sleep(1);
    }

    debugDraw();
}

void Shape::getCracksOutside(vector<Crack*> *toFill)
{
    vector<Crack*>::iterator c;
    for (c = cracks.begin(); c != cracks.end(); ++c)
    {
        if (!inside((*c)->point2))
        {
            toFill->push_back(*c);
        }
    }
}

void Shape::removeCracksOutside()
{
    vector<Crack*>::iterator c;
    
    c = cracks.begin();
    while (c != cracks.end())
    {
        if ((rayTrace(**c) % 2) == 0) //if even, erase
            //if (!inside((*c)->point2))
        {
            c = cracks.erase(c);
        }
        else
        {
            ++c;
        }
    }
}

bool Shape::inside(Point toTest)
{
    Line ray(toTest, 42, 45);// rand() % 360);
    int rayResult = rayTrace(ray);
    return (rayResult % 2); //if odd, inside, if even, outside.
}

string Shape::generateJSON() const
{
    string toReturn;
    toReturn = "{\"lines\":[";
    vector<Line*>::const_iterator l;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        toReturn += (*l)->generateJSON();
        if (l + 1 != lines.end())
        {
            toReturn += ",";
        }
    }
    toReturn += "]";
    toReturn += ",\"cracks\":[";
    vector<Crack*>::const_iterator c;
    for (c = cracks.begin(); c != cracks.end(); ++c)
    {
        toReturn += (*c)->generateJSON();
        if (c + 1 != cracks.end())
        {
            toReturn += ",";
        }
    }
    toReturn += "]";
    stringstream streamy;
    streamy << ",\"speed\":" << speed
            << ",\"direction\":" << direction
            << ",\"angularVelocity\":" << angularVelocity
            << ",\"orientation\":" << orientation
            << ",\"xMinBound\":" << xMinBound
            << ",\"xMaxBound\":" << xMaxBound
            << ",\"yMinBound\":" << yMinBound
            << ",\"yMaxBound\":" << yMaxBound;
        
    streamy << ",\"bound\":";
    switch (this->bound)
    {
    case SHAPE_BOUND_NONE:
        streamy << "\"none\"";
        break;
    case SHAPE_BOUND_BOUNCE:
        streamy << "\"bounce\"";
        break;
    case SHAPE_BOUND_DESTROY:
        streamy << "\"destroy\"";
        break;
    case SHAPE_BOUND_WRAP:
        streamy << "\"wrap\"";
        break;
    }
    toReturn += streamy.str();
    toReturn += "}";
    return toReturn;
}

void Shape::save(string fileName)
{
    ofstream outFile(fileName.c_str());
    outFile << generateJSON();
    outFile.close();
}

//JSON Notes: no whitespace. All list objects encapsulated in {}.
//Also: for cracks, startLine and parentShape are regenerated.
//If the save of the crash works, look into those.
void loadLines(string fileName, vector<Line*> *toFill)
{
    vector<string> jsonLines;
    vector<string>::iterator i;
    string jsonString;

    //load the file into the string
    ifstream inFile(fileName.c_str());
    getline(inFile, jsonString);
   
    //get all the json objects into a vector
    parseJsonList(&jsonString, &jsonLines);

    //create the lines from the json objects
    for (i = jsonLines.begin(); i != jsonLines.end(); ++i)
    {
        toFill->push_back(new Line(*i));
    }
    /*
    char nextChar;
    string lineObject;
    do 
    {
        nextChar = jsonString[0];
        if (nextChar == '{')
        {
            lineObject = grabJsonObject(&jsonString);
            cout << lineObject << endl;
            //toFill->push_back(new Line(lineObject));
            toFill->push_back(new Line(Point(100, 100), 100, 45));
        }
        else
        {
            jsonString.erase(0, 1);
        }
    } while (nextChar != ']');
    */
}

Shape *loadShape(string filename, vector<Shape*> *toDraw)
{
    ifstream inFile(filename.c_str());
    string jsonString;
    getline(inFile, jsonString);
   
    return new Shape(jsonString, toDraw);
}

void loadShapes(string filename, vector<Shape*> *toDraw)
{
    ifstream inFile(filename.c_str());
    if (inFile.fail())
    {
        cerr << "Invalid file name!" << endl;
        return;
    }
    string jsonString;
    getline(inFile, jsonString);
    vector<string> shapeJsons;
    parseJsonList(&jsonString, &shapeJsons);
    vector<string>::iterator i;
    for (i = shapeJsons.begin(); i != shapeJsons.end(); ++i)
    {
        toDraw->push_back(new Shape(*i, toDraw));
    }
}

void saveShapes(string filename, vector<Shape*> *toDraw)
{
    string toSave;
    vector<Shape*>::iterator s;
    toSave = "[";
    for (s = toDraw->begin(); s != toDraw->end(); ++s)
    {
        toSave += (*s)->generateJSON();
        if (s + 1 != toDraw->end())
        {
            toSave += ",";
        }
    }
    toSave += "]";
    ofstream outFile(filename.c_str());
    outFile << toSave;
    outFile.close();
}

int Shape::saveNum = 0;

Shape *Shape::getShapeWithCrack(Crack *crackToCheck)
{
    vector<Crack*>::iterator c;
    vector<Shape*>::iterator s;
    for (s = toDraw->begin(); s != toDraw->end(); ++s)
    {
        for (c = (*s)->cracks.begin(); c != (*s)->cracks.end(); ++s)
        {
            if (crackToCheck == *c)
            {
                return *s;
            }
        }
    }
    return NULL;
}

/*
//expects that each child it is passed is a point crack.
//randomly adds another crack (or not) to each child.
void Shape::splitChildren(vector<Crack*> *children)
{
    vector<Crack*> childCopy;
    childCopy.assign(children->begin(), children->end());
    
    children->clear();
    vector<Crack*>::iterator c;
    double percentChance = 42;
    for (c = childCopy.begin(); c != childCopy.end(); ++c)
    {
        children->push_back(*c);
        if (JDL::randDouble(0, 100) < percentChance)
        {
            //add a child to the parent of *c
            Crack *theParent = (*c)->getParentCrack();
            if (theParent != NULL)
            {
                children->push_back(theParent->addChild());
            }
            else
            {
                Crack *toAdd = new Crack((*c)->point1);
                cracks.push_back(toAdd);
                children->push_back(toAdd);
            }
        }
    }
}
*/


//if children is one (and a point), do nothing.
//Otherwise, for each child, either create two new points for it or do nothing.
void Shape::splitChildren(vector<Crack*> *children)
{
    if ((*children)[0]->isPoint()) return;

    vector<Crack*> childCopy;
    childCopy.assign(children->begin(), children->end());
    
    children->clear();
    vector<Crack*>::iterator c;
    double percentChance = 33.33;

    for (c = childCopy.begin(); c != childCopy.end(); ++c)
    {
        if ((*c)->isShapeSplit())
        {
            //do nothing, do not add to children; do not increase this crack.
            cerr << "Ignoring expanding Crack with shapeSplit == true." << endl;
        }
        else if (JDL::randDouble(0, 100) < percentChance)
        {
            //give it two children
            children->push_back((*c)->addChild());
            children->push_back((*c)->addChild());
        }
        else
        {
            //give it one child
            children->push_back((*c)->addChild());
        }
    }
}

