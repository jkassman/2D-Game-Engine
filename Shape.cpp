#include "Shape.hpp"
#include "JDL.hpp"
#include "jacobJSON.hpp"

#define _USE_MATH_DEFINES

#include <iostream>
#include <algorithm>
#include <math.h>
#include <fstream>
#include <sstream>

using namespace std;

//corners are 0-3;
int Shape::newestID = 4;

void Shape::initNull()
{
    this->bound = SHAPE_BOUND_NONE;
    this->projectile = false;
    this->speed = 0;
    this->direction = 0;
    this->xMinBound = 0;
    this->xMaxBound = 400;
    this->yMinBound = 0;
    this->yMaxBound = 400;
   
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
}

//NOTE: Just copies the pointers into lines vec. does not call constructor.
//does call build, though.
void Shape::updateLines(const vector <Line*> &newLines)
{
    //TODO: Actually delete the old lines
    lines.clear();
    vector<Line*>::const_iterator l;
    for (l = newLines.begin(); l != newLines.end(); ++l)
    {
        lines.push_back(*l);
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
    string linesString = grabJsonValue(jsonString, "lines");
    string cracksString = grabJsonValue(jsonString, "cracks");
    
    this->speed = JDL::stringToDouble(speedString);
    this->direction = JDL::stringToDouble(directionString);
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
    //calculateCenter();
    build();
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
}

 void Shape::move(double distance, double degrees)
 {
     vector<Line*>::iterator l;
     for (l = lines.begin(); l != lines.end(); ++l)
     {
         (*l)->move(distance, degrees);
     }
     vector<Crack*>::iterator c;
     for (c = cracks.begin(); c != cracks.end(); ++c)
     {
         (*c)->move(distance, degrees);
     }
     center.move(distance, degrees);
     boxTopLeft.move(distance, degrees);
 }

 void Shape::move()
 {
     move(speed, direction);
 }

 void Shape::setVelocity(double speed, double degrees)
 {
     this->speed = speed;
     this->direction = degrees;
 }

double Shape::getSpeed()
{
    return speed;
}

double Shape::getDirection()
{
    return direction;
}

 void Shape::draw()
 {
     /*
     Line sideLine(boxTopLeft, Point(boxTopLeft.x, boxTopLeft.y + height));
     Line topLine(boxTopLeft, Point(boxTopLeft.x + width, boxTopLeft.y));
     sideLine.draw();
     topLine.draw();
     */
     center.drawCircle(5);
     vector<Line*>::iterator l;
     for (l = lines.begin(); l != lines.end(); ++l)
     {
         (*l)->draw();
     }
     vector<Crack*>::iterator c;
     for (c = cracks.begin(); c != cracks.end(); ++c)
     {
         ((Line*)(*c))->draw(c - cracks.begin());
         (*c)->draw();
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

void Shape::collide(Shape *collider)
{
    if (collider->ID != lastHit)
    {
        double m2;
        m2 = collider->mass;

        //changes the value of the velocities!
        double myVelX, myVelY;
        double collideVelX, collideVelY;
        myVelX = cos(direction *M_PI/180) * speed;
        myVelY = sin(direction *M_PI/180) * speed;
        collideVelX = cos(collider->direction*M_PI/180) * collider->speed;
        collideVelY = sin(collider->direction*M_PI/180) * collider->speed;
        collide1D(this->mass, m2, myVelX, collideVelX);
        collide1D(this->mass, m2, myVelY, collideVelY);

        Line dir(Point(0,0), Point(myVelX, myVelY));
        direction = dir.getDirection();
        speed = dir.length();

        Line dir2(Point(0,0), Point(collideVelX, collideVelY));
        collider->direction = dir2.getDirection();
        collider->speed = dir2.length();

        lastHit = collider->ID;
        collider->lastHit = ID;
        //std::cout << "ball " << myId << " collided with ball " 
        //          << collider.myId << std::endl;
    }
    else
    {
        //std::cout << "Skipping collide of ball " << myId 
        //          << " with ball " << collider.myId << std::endl;
    }
}
	
//My current (2016) thought is that something is wrong in here
//for some reason, on some occassions, a collision occurs, but nothing
//happens; there is no change in velocity.
//this triggers the lastHit mechanism to let the balls pass through each other.
void collide1D(double m1, double m2, double &v1i, double &v2i)
{
	//helpful constants
	double c1, c2;
	c1 = (m1 * v1i) + (m2 * v2i);
	c2 = (m1 * (v1i * v1i)) + (m2 * (v2i * v2i));

	//std::cout << m1 << " " << m2 << " " << v1i << " " << v2i << std::endl;

	//actual calculation:
	double a, b, c;
	a = (m2 * m2) + (m1 * m2);
	b = -2 * c1 * m2;
	c = (c1 * c1) - (c2 * m1);

	//quadratic equation:
	//double double_error = 0;
	double toRoot = (b*b) - (4 * a*c);
	if (toRoot < 0)
	{
		std::cout << "Imaginary number!" << std::endl;
	}
	double v2f = ((-b) + sqrt(toRoot)) / (2 * a);
	//Ignore the no collision answer:
	//THIS PART IS FAILING SOMETIMES! (?) //OH!! ONE DIMENSIONAL!
        double double_error = 0.001;
	if ((v2f >= v2i - double_error) && (v2f <= v2i + double_error))
            //if (v2f == v2i)
	{
		v2f = ((-b) - sqrt(toRoot)) / (2 * a);
	}

	//change the values of the parameters!
	
	v2i = v2f;
	v1i = (c1 - (m2*v2f)) / m1;

	if (isnan(v2i) || isnan(v1i))
	{
		std::cout << "Found a nan in collision" << std::endl;
	}
}


void Shape::collide()
{
    vector<Shape*>::iterator s;
    vector<Line*>::iterator l;
    for (s = toDraw->begin(); s != toDraw->end(); ++s)
    {
        if ((*s) == this) continue;
        //if ((lastHit == (*s)->ID) && ((*s)->lastHit == ID)) continue;
        //if ((*s)->ID == ID) continue;
        if (this->hitBoxOverlapsWith(**s))
        {
            for (l = this->lines.begin(); l != this->lines.end(); ++l)
            {
                Point crashPoint = (*l)->point1;
                if ((*s)->inside(crashPoint))
                {
                    collide(*s);
                    /*
                    if ((rand() %10) == 8)
                    {
                        fractureAt(crashPoint);
                    }
                    */
                    /*
                    newestID++;
                    this->ID = newestID;
                    newestID++;
                    (*s)->ID = newestID;
                    
                    this->lastHit = (*s)->ID;
                    (*s)->lastHit = this->ID;

                    speed = -speed;
                    (*s)->speed = -(*s)->speed;
                    
                    fractureAt(crashPoint);
                    (*s)->fractureAt(crashPoint);
                    */
                    return;
                }
            }
        }
    }
/*
    if (!projectile) return;
    
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        //for all other shapes
        for (s = toDraw->begin(); s != toDraw->end(); ++s)
        {
            if (*s == this) continue;
            if ((*s)->inside((*l)->point1))
            {
                Point crashPoint = (*l)->point1;
                direction += 180;
                crashPoint.drawCircle(3);
                //fractureAt(crashPoint);
                projectile = false;
                //(*s)->direction = (*s)->direction;
                (*s)->fractureAt(crashPoint);
                return;
            }
        }
    }
*/
//#endif
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
    double xMagnitude = cos(M_PI/180.0 * direction) * speed;
    double yMagnitude = sin(M_PI/180.0 * direction) * speed;
    switch (corner)
    {
    case SHAPE_CORNER_LEFT:
        //make the shape go right
        if (xMagnitude < 0) xMagnitude = -xMagnitude;
        else cout << "Already going right, skipping" << endl;
        lastHit = -1;
        break;
    case SHAPE_CORNER_RIGHT:
        //make the shape go left;
        if (xMagnitude > 0) xMagnitude = -xMagnitude;
        else cout << "Already going left, skipping" << endl;
        lastHit = -2;
        break;
    case SHAPE_CORNER_UP:
        //make the shape go down;
        if (yMagnitude < 0) yMagnitude = -yMagnitude;
        else cout << "Already going down, skipping" << endl;
        lastHit = -3;
        break;
    case SHAPE_CORNER_DOWN:
        //make the shape go up;
        if (yMagnitude > 0) yMagnitude = -yMagnitude;
        else cout << "Already going up, skipping" << endl;
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

#if 0
    if (xMagnitude < 0)
    {
        cout << "I bet you will never guess who this is." << endl;
        cout << "OH MY GOD IT'S THE MATRIX YES I'M NEO" << endl;
    }
#endif
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

/*
 Crack *Shape::addCrack(Point impactPoint)
 {
     vector<Line*>::iterator l;
     Point result;
     for (l = lines.begin(); l != lines.end(); ++l)
     {
         if ((*l)->on(impactPoint, 10, &result))
         {
             return (*l)->addCrack(result, this);
         }
     }
     return NULL;
 }
*/
 //returns how many cracks were affected
 int Shape::fractureAt(Point clickPoint)
 {
     vector<Crack*> children;
     vector<Line*> splitLines;
     vector<Crack*> splitCracks;
     Point impactPoint;

     double force = 50;
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
         //(*c)->increaseOne(force/children.size());
         (*c)->expand(force, this);
     }

     /*
     for (c = children.begin(); c != children.end(); ++c)
     {
         if ((*c)->isShapeSplit())
         {
             splitLines.clear();
             (*c)->getSplitLinesAndCracks(&splitLines, &splitCracks, this);
             getShapeWithCrack(*c)->split(splitLines, splitCracks);
         }
     }
     */

     return children.size();
}

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
}

/*
Line *Shape::getLineNearest(Point clickPoint) const
{
    vector<Line*>::const_iterator l;
 
    double radius = 1000000;
   
    Line *toReturn = NULL;

    double minDist = Line(clickPoint, lines.front()->point1).length();
    toReturn = lines.front();
    Point resultPoint;
    
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        (*l)->on(clickPoint, radius, &resultPoint);
        Line dist(clickPoint, resultPoint);
        if (dist.length() < minDist)
        {
            minDist = dist.length();
            toReturn = *l;
        }
    }
    
    return toReturn;
}

//Return the line that:
//has the nearest crack within radius
//or, if no line has a crack within radius, the first line found.
Crack * Shape::getAffectedCrack(Point impactPoint, double radius)
{
    //step 1: find the line that impactPoint is on.
    vector<Line*>::const_iterator l, lon = lines.end();
    if (!lines.size())
    {
        cerr << "WTF...Problems!" << endl;
        return NULL;
    }
    Line *lineOn = getLineNearest(impactPoint);
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l)->on(impactPoint))
        {
            lon = l;
            break;
        }
    }
    if (lon == lines.end())
    {
        impactPoint.drawCircle(3);
        JDL::flush();
        JDL::sleep(5);
        cerr << "ERROR: The impactPoint was not on the shape" << endl;
        return NULL;
    }
    //step 2: find the crack on lon with the closest start point to impactPoint
    Line distance;
    distance.point1 = impactPoint;
    double minDist = radius;
    vector<Crack*>::const_iterator c;
    Crack *toReturn = NULL;
    for (c = (*lon)->cracks.begin(); c!= (*lon)->cracks.end(); ++c)
    {
        distance.point2 = (*c)->startPoint();
        if (distance.length() < minDist)
        {
            minDist = distance.length();
            toReturn = *c;
        }
    }

    //step 3: search backwards through the lines to find a crack.
    double backwardsDist = Line(impactPoint, (*lon)->point1).length();
    l = lon;
    while (backwardsDist < radius)
    {
        if (l == lines.begin())
        {
            l = lines.end();
        }
        l--;

        distance.point1 = (*l)->point2;
        for (c = (*l)->cracks.begin(); c!= (*l)->cracks.end(); ++c)
        {
            distance.point2 = (*c)->startPoint();
            if ((distance.length() + backwardsDist)< minDist)
            {
                minDist = distance.length() + backwardsDist; 
                toReturn = *c;
            }
        }

        backwardsDist += (*l)->length();
    }

    //Step 4: search forwards through the lines to find a crack.
    double forwardsDist = Line(impactPoint, (*lon)->point2).length();
    l = lon;
    while (forwardsDist < radius)
    {
        l++;
        if (l == lines.end())
        {
            l = lines.begin();
        }

        distance.point1 = (*l)->point1;
        for (c = (*l)->cracks.begin(); c!= (*l)->cracks.end(); ++c)
        {
            distance.point2 = (*c)->startPoint();
            if ((distance.length() + forwardsDist) < minDist)
            {
                minDist = distance.length() + forwardsDist; 
                toReturn = *c;
            }
        }

        forwardsDist += (*l)->length();
    }
    
    if (!toReturn)
    {
        //create new crack on lon
        toReturn = (*lon)->addCrack(impactPoint, this);
    }
    return toReturn;
}
*/
 
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
    /*
    Crack *toReturn = NULL;
    if (closestCrack == NULL)
    {
        toReturn = new Crack(impactPoint);
        cracks.push_back(toReturn);
    }
    else
    {
        toReturn = closestCrack->addChild();
    }
    */
    if (closestCrack == NULL)
    {
        closestCrack = new Crack(impactPoint);
        cracks.push_back(closestCrack);
    }
    return closestCrack;
}

//assume forces and hitPoints are empty.
//fills both of them to correspond to lines.
void Shape::distributeForce(Point impactPoint, double force, double radius,
                            std::vector<int> *forces, 
                            std::vector<Point> *hitPoints) const
{
    
    /*
      In this iteration, all we do is check if the impact point is near
      the corner. If it is, split up the impact.
      Eventually, look at force and size of the line and stuff
      Ya, looking at that stuff is starting to drive me crazy.
    */ 

    //ok, so what do we want?
    //loop through all the lines.
    //When we find the one that it is on, do stuff.
    //if corner, we have some problems...
    //put half the force on this one
    //make a new function to distribute force?
    //or is that this function?

    //that's this function!

    //scan through 
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

bool Shape::pointOn(Point toCheck, double radius, Point *resultPoint) const
{
    vector<Line*>::const_iterator l;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l)->on(toCheck, radius, resultPoint))
        {
            return true;
        }
    }
    return false;
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

/*
    //remove any point1 intersects:    
    for (p = intersectPoints.begin(); p != intersectPoints.end();)
    {
        if (*p == toCheck.point1)
        {
            p = intersectPoints.erase(p);
            c = intersectCracks.erase(c);
        }
        else
        {
            ++c;
            ++p;
        }
    }
*/

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
            cout << "WOO" << endl;
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
    vector<Point> intersectPoints;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        if (ray.rayIntersects(**i, &intersectPoint))
        {
            intersectPoints.push_back(intersectPoint);
            numIntersects++;
        }
    }
    
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
                numIntersects--;
            }
            previous = *j;
        }
    }
    return numIntersects;
}

//the crack should be destroyed after a split
void Shape::split(const vector<Line*> &splitLines, 
                  const vector<Crack*> &splitCracks)
{
    //new new plan:
    //split the shape by lines, create two new shapes from them.
    //split startLine, split endLine, then throw the lines into shapes.

    //STUPID DEBUGGING
    

    //endLine splits intersectPoint->point2.
    vector<Line*> lines1, lines2;
    vector<Line*>::const_iterator l;
    vector<Crack*>::iterator c;
    
    Point crackStartPoint, crackEndPoint;
    crackStartPoint = splitLines[0]->point1;
    crackEndPoint = splitLines.back()->point2;
    if (crackStartPoint == crackEndPoint)
    {
        cerr << "PROBLEMS!!!!" << endl;
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
#if 1
    for (l = splitLines.begin(); l != splitLines.end(); ++l)
    {
        splitLinesCopy1.push_back(new Line(**l));
        splitLinesCopy2.push_back(new Line(**l));
    }
#endif
/*
    //but only copy the lines properly. Copy the cracks as pointers.
    for (l = splitLines.begin(); l != splitLines.end(); ++l)
    {
        splitLinesCopy1.push_back(new Line((*l)->point1, (*l)->point2));
        for (c = (*l)->cracks.begin(); c != (*l)->cracks.end(); ++c)
        {
            splitLinesCopy1.back()->cracks.push_back(*c);
        }
        splitLinesCopy2.push_back(new Line((*l)->point1, (*l)->point2));
        for (c = (*l)->cracks.begin(); c != (*l)->cracks.end(); ++c)
        {
            splitLinesCopy2.back()->cracks.push_back(*c);
        }
    }
*/  
    appendLines(&lines1, splitLinesCopy1);
    appendLines(&lines2, splitLinesCopy2);
    
    /*
      Finished creating the new lines for the shapes.
      Time to make shapes from these lines:
    */
    /* Modify this shape (using lines2) */
    //reassign the lines in this shape to be those of lines2.
    /*
    this->lines.clear();
    this->lines.assign(lines2.begin(), lines2.end());
    calculateCenter();
    */
    this->updateLines(lines2);

    vector<Crack*> outsideCracks;
    getCracksOutside(&outsideCracks);
    addCracks(splitCracks);
    removeCracksOutside();

    if (!sanityCheck())
    {
        debugDraw();
    }


    /* Create a new shape (using lines1) */
    Shape *newShape = new Shape(lines1, this->toDraw);
    newShape->copyBounds(*this);
    //newShape->calculateCenter();
    newShape->addCracks(splitCracks);
    newShape->addCracks(outsideCracks);
    newShape->removeCracksOutside();

    if (!newShape->sanityCheck())
    {
        newShape->debugDraw();
    }
    toDraw->push_back(newShape);
    newShape->speed = this->speed;
    newShape->direction = this->direction;

    //make the new shape move away from the current shape.
    int newDirection = 0;    
    Line crackDirection(splitLines[0]->point1, splitLines.back()->point2);
    
    int modifier = 1;
    //if (sameLineSplit) modifier = -1;
    newDirection = crackDirection.getDirection() + (modifier*90);
    //newShape->setSpeed(0.42 + this->speed, newDirection);
    newShape->accelerate(0.42, newDirection);

    //TODO: Delete splitlines here! We made two copies
    
    cout << "Finished the split" << endl;
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

/*
void Shape::updateCrackParents()
{
    vector<Line*>::iterator l;
    for (l = this->lines.begin(); l != this->lines.end(); ++l)
    {
        (*l)->setCrackParents(this);
    }
}
*/

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
    */
#if 0
    startPoint.print();
    startPoint.drawCircle(3);
    JDL::flush();
    JDL::sleep(5);
#endif
    vector<Line*>::iterator l;
    //should break before reaching lines.end().
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l)->on(startPoint))
        {
            if (startPoint == (*l)->point1)
            {
                break;
            }
            else if (startPoint == (*l)->point2)
            {
                //no split required, nothing to push back.
                ++l;
                break;
            }
            else
            {
                Line *newLine = new Line();
                (*l)->split(startPoint, newLine);
                l = lines.insert(l+1, newLine);
                //result->push_back(newLine);
                //newLine->index = 0;
                break;
            }
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
    //int index = 0;
    
    //note: we may have to wrap around the shape here
    //the only exit is via breaks. yay standards.
    while (true)
    {
        if ((*l)->on(endPoint))
        {
            if (endPoint == (*l)->point1)
            {
                //no split required, don't push anything back.
                break;
            }
            else if (endPoint == (*l)->point2)
            {
                result->push_back(*l);
                //(*l)->index = index;
                break;
            }
            else
            {
                Line *newLine = new Line();
                (*l)->split(endPoint, newLine);
                //(*l)->index = index;
                result->push_back(*l);
                lines.insert(l+1, newLine);
                break;
            }
        }
        result->push_back(*l);
        //(*l)->index = index;
        //index++;
        ++l;
        if (l == lines.end())
        {
            l = lines.begin();
        }
    }
}

void appendLines(std::vector<Line*> *lines1, std::vector<Line*> &lines2)
{    

    Line *lastLine1 = lines1->back();
    //int index = lastLine1->index + 1;

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
            //(*i)->index = index;
            //index++;
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
            //(*i)->index = index;
            //index++;

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
            //int tempIndex = (*debug)->index;
            //(*debug)->index = 1;
            (*debug)->draw(1);
            //(*debug)->index = tempIndex;
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

void Shape::accelerate(double acceleration, double degrees)
{
    cout << "Speed: " << this->speed << endl;
    Point origin(0,0);
    Line velocity(origin, this->speed, this->direction);
    velocity.point2.x += acceleration * cos(M_PI/180 * degrees);
    velocity.point2.y += acceleration * sin(M_PI/180 * degrees);
    
    this->direction = velocity.getDirection();
    this->speed = velocity.length();
    cout << "New Speed: " << this->speed << endl;
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

//assumes that the hitbox is set up already.
void Shape::calculateMass()
{
    int totalX = 0, totalY = 0;
    int w = 0, h = 0;
    mass = 0;
    for (w = boxTopLeft.x; w < boxTopLeft.x + width; w++)
    {
        for (h = boxTopLeft.y; h < boxTopLeft.y + height; h++)
        {
            if (inside(Point(w, h)))
            {
                //cout << w << "," << h << endl;
                /*
                JDL::setDrawColor(255, 0, 255);
                JDL::point(w, h);
                JDL::setDrawColor(255, 255, 255);
                */
                //JDL::circle(w, h, 5);
                totalX += w;
                totalY += h;
                mass += 1;
            }
        }
    }
    cout << "totalX: " << totalX << ", mass: " << mass << endl;
    //Note: This trick only works with mass += 1 every time.
    center = Point(totalX/mass, totalY/mass);
    center.print();
}
/*
void Shape::calculateCenter()
{
    vector<Line*>::iterator l;
    double xTotal = 0;
    double yTotal = 0;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        xTotal += (*l)->point2.x;
        yTotal += (*l)->point2.y;
    }
    center.x = xTotal/lines.size();
    center.y = yTotal/lines.size();
    estRadius = 0;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        Line dist(center, (*l)->point2);
        if (dist.length() > estRadius)
        {
            estRadius = dist.length();
        }
    }
    }*/

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
        if ((rayTrace(*(Line*)*c) % 2) == 0) //if even, erase
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
    Line ray(toTest, 42, 0);
    int rayResult = rayTrace(ray);
    return (rayResult % 2); //if odd, inside, if even, outside.
}

/*
string Shape::generateJSON()
{
    string toReturn;
    //stringstream streamy;
    //streamy << "\"Shape" << index << "\": {";
    //toReturn = streamy.str();
    toReturn = "[";
    vector<Line*>::iterator l;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        toReturn += (*l)->generateJSON();
        if (l + 1 != lines.end())
        {
            toReturn += ",";
        }
    }
    toReturn += "]";
    return toReturn;
}
*/

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

