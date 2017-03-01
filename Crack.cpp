#include "Crack.hpp"
#include "JDL.hpp"

#define _USE_MATH_DEFINES

#include <math.h>
#include <algorithm>
#include <iostream>

using namespace std;

void Crack::init(Shape *parentShape, Point startPoint, Line *startLine)
{
    this->shapeSplit = false;
    this->doDelete = false;
    this->intersectCrack = NULL;
    
    this->startLine = startLine;
    this->parentShape = parentShape;
    addPoint(startPoint);
}

Crack::Crack(Shape *parentShape, Point startPoint, Line *startLine)
{
    init(parentShape, startPoint, startLine);
}

Crack::Crack(Shape *parentShape, const vector<Line*> &newLines, Line *start)
{
    init(parentShape, newLines[0]->point1, start);
    vector<Line*>::const_iterator l;
    for (l = newLines.begin(); l != newLines.end(); ++l)
    {
        addPoint((*l)->point2);
    }
}

Crack::Crack(const Crack &other)
{   
    this->intersectCrack = other.intersectCrack;
    this->doDelete = other.doDelete;
    this->startLine = other.startLine;
    this->parentShape = other.parentShape;
    this->point = other.point;
    this->shapeSplit = other.shapeSplit;
    
    //std::vector<Line*> lines;
    std::vector<Line*>::const_iterator l;
    for (l = other.lines.begin(); l != other.lines.end(); ++l)
    {
        this->lines.push_back(new Line(**l));
    }
}

void Crack::increase(double force)
{
    if (shapeSplit)
    {
        cerr << "Cannot increase crack that has already split the shape!"
             << endl;
        return;
    }

    //New Idea:
    //Create lines with random slopes starting from the given point until
    //One of those lines points towards the inside of the shape
    
    //Then, see if that line intersects with the edge of the shape
    //    (Or maybe even another crack)
    //If it does, split the shape apart.
    //    (If it intersects with multiple, choose the closest edge)

    int numIntersects = 0;
      
    vector<Point> intersectPoints;
    vector<Line*> intersectLines;
    Line fractureLine;
    double turnDegrees = 110;

    while (true)
    {
        double degree;
        
        //create a random slope of the line
        if (point)
        {
            degree = JDL::randDouble(0, 360);
            //degree = rand() % 360;
        }
        else
        {
            double direction = lines.back()->getDirection();
            double modifier = JDL::randDouble(-turnDegrees/2, turnDegrees/2);
            //modifier = 25; //temporary debug lock
            degree = direction + modifier;
        }


        fractureLine = Line(lines.back()->point2, force, degree);
        JDL::setDrawColor(255, 0, 0);
        fractureLine.draw();
        JDL::setDrawColor(255,255,255);
        JDL::flush();

        bool badLine = false;
        vector<Line*>::iterator l;
#ifdef AVOID_OTHERS
        //if it intersects with a crack, choose a new line.
        vector<Crack*>::iterator c;
        //for all lines
        
        for (l = parentShape->lines.begin(); l != parentShape->lines.end(); ++l)
        {
            for (c = (*l)->cracks.begin(); c != (*l)->cracks.end(); ++c)
            {
                if ((*c) == this) break; //exempt self intersection for now.
                if ((*c)->lineIntersects(fractureLine))
                {
                    badLine = true;
                }
            }
        }
#else
        //if it intersects itself, choose a new line
        Point selfIntersect;
        for (l = lines.begin(); l != lines.end(); ++l)
        {
            if ((*l)->intersects(fractureLine, &selfIntersect))
            {
                if (selfIntersect != fractureLine.point1)
                {
                    badLine = true;
                }
            }
        }
#endif
        if (badLine) 
        {
//            JDL::sleep(1);
            turnDegrees += 10;
            if (turnDegrees > 360)
            {
                turnDegrees = 360;
            }
            continue;
        }

        

        //check if the line is on top of one of the shape lines, and reject it
        //if it is.
        if (parentShape->lineOnBorder(fractureLine))
        {
            JDL::setDrawColor(255, 0, 255);
            fractureLine.draw();
            JDL::setDrawColor(255,255,255);
            JDL::flush();
            continue;
        }

        //see how many lines fractureLine's ray intersects with.
        //(make sure the crack is in the right direction)
        int numRay = parentShape->rayTrace(fractureLine);

        //if the number lines intersected is odd, it's in the right direction.
        //else, repeat this process again.
        if (numRay % 2) //if odd
        {
            break;
        }
//        JDL::sleep(1);
    }
    //Now we know the crack is in the right direction.
    //But we need to count how many shape lines it intersects
    //(if it intersects no lines, we're done)

    //First check all the shape lines
    vector<Line*>::iterator i;
    Point intersectPoint;
    for (i = parentShape->lines.begin(); i != parentShape->lines.end(); ++i)
    {
        if ((*i)->intersects(fractureLine, &intersectPoint))
        {
            if (intersectPoint == fractureLine.point1)
            {
                continue;
            }
            numIntersects++;
            intersectPoints.push_back(intersectPoint);
            intersectLines.push_back(*i);
        }
    }
    
    //Then check all the crack lines
    vector<Crack*> intersectCracks;
    numIntersects += parentShape->lineIntersectsCrack(fractureLine, 
                                                      &intersectCracks, 
                                                      &intersectPoints,
                                                      this);

    Line *shapeLine = NULL;
    if (numIntersects > 0)
    {
        shapeSplit = true;
        fractureLine.point2 = intersectPoints[0];
        if (intersectLines.size() != 0)
        {
            shapeLine = intersectLines[0];
        }
        else
        {
            //Note: this is a private variable
            intersectCrack = intersectCracks[0];
        }
        //TODO: handle case where numIntersects > 1!
    }
/*
    if (numIntersects > 0)
    {
        fractureLine.point2 = intersectPoints[0];
        shapeLine = intersectLines[0];
        if (numIntersects > 1)
        {
            //if there are multiple intersects, choose the closest edge
            vector<Point>::iterator j;
            vector<Line*>::iterator shapeLineIter = intersectLines.begin();
            for (j = intersectPoints.begin()+1; j != intersectPoints.end(); ++j)
            {
                shapeLineIter++;
                Line checkLine(fractureLine.point1, *j);
                if (checkLine.length() < fractureLine.length())
                {
                    fractureLine.point2 = *j;
                    shapeLine = *shapeLineIter;
                }
            }
        }
        shapeSplit = true;
    }
*/
    addPoint(fractureLine.point2);
    cout << "exiting increase()" << endl;
    JDL::setDrawColor(0, 255, 0);
    fractureLine.draw();
    JDL::setDrawColor(255,255,255);
    JDL::flush();
}

void Crack::move(double distance, double degrees)
{
    vector<Line*>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        (*i)->move(distance, degrees);
    }
}

void Crack::draw() const
{
    vector<Line*>::const_iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        (*i)->draw(); //oo this looks like a possible stack overflow :)
        //(Look at Line's draw. it calls this draw).
    }
}

Point Crack::startPoint()
{
    return lines[0]->point1;
}

void Crack::addPoint(Point toAdd)
{
    //cout << "size: " << lines.size() << endl;
    if (!lines.size())
    {
        point = true;
        lines.push_back(new Line(toAdd, toAdd));
    }
    else if (point)
    {
        lines[0]->point2 = toAdd;
        point = false;
    }
    else
    {
        lines.push_back(new Line(lines.back()->point2, toAdd));
    }
    lines.back()->index = lines.size()-1;
}


void Crack::clearLines()
{
    lines.clear();
}

void Crack::getSplitLines(vector<Line*> *splitLines)
{
    vector<Line*>::iterator l;
    
    //first, get the split lines from the intersectCrack
    vector<Line*> intersectCrackLines;
    vector<Line*> newCrack;
    bool afterIntersect = false;
    if (intersectCrack)
    {
        cout << "intersectCrack is true!" << endl;
        for (l = intersectCrack->lines.begin();
             l != intersectCrack->lines.end(); ++l)
        {
            if (!afterIntersect)
            {
                intersectCrackLines.push_back(*l);
            }
            else
            {
                cout << "ERE" << endl;
                newCrack.push_back(*l);
            }
            if ((*l)->on(lines.back()->point2))
            {
                //TODO: handle (literal) corner cases

                //Split the line.
                //put half the line on splitLines,
                //then put the other half and the rest of the crack on a crack
                //on the last line of this crack.
                Line *newLine = new Line();
                (*l)->split(lines.back()->point2, newLine);
                newCrack.push_back(newLine);
                afterIntersect = true;
            }
        }
    }
/*
    JDL::setDrawColor(255, 0, 255);
    drawLines(intersectCrackLines, 0);
    JDL::setDrawColor(255,255,255);
    JDL::sleep(5);
*/
   
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        splitLines->push_back(*l);
    }

   
    
    //merge and resolve the two lines of cracks.
    //we have splitLines, which is this crack's lines
    //and intersectCrackLines, which is the lines the other crack contributed.
    if (intersectCrack)
    {
        //Step 1: flip intersectCrackLines (and add it to splitLines)
        vector<Line*>::reverse_iterator rl;
        for (rl = intersectCrackLines.rbegin();
             rl != intersectCrackLines.rend(); ++rl)
        {
            (*rl)->switchPoints();
            splitLines->push_back(*rl);
        }

        //Step 2: delete intersectCrack from its line
        intersectCrack->startLine->deleteCrack(intersectCrack);

        //Step 3: reassign newCrack to lines.back()
        intersectCrack->lines.assign(newCrack.begin(), newCrack.end());
        lines.back()->cracks.push_back(intersectCrack);

        /*S*DF*SDF**/
        //Step 2: add newCrack to lines.back()
        //lines.back()->cracks.push_back(new Crack(parentShape, newCrack, 
        //lines.back()));

        //Step 3: Mark intersectCrack for deletion
        //intersectCrack->doDelete = true;
/*
        JDL::setDrawColor(0, 255, 255);
        drawLines(*splitLines, 0);
        JDL::setDrawColor(255,255,255);
        JDL::sleep(5);
*/

    }

}

bool Crack::isShapeSplit()
{
    return shapeSplit;
}

bool Crack::shouldDelete()
{
    return doDelete;
}

void Crack::setParent(Shape *parentShape)
{
    this->parentShape = parentShape;
}

Line *Crack::getStartLine()
{
    return this->startLine;
}

bool Crack::lineIntersects(const Line &toCheck, Point *intersect)
{
    vector<Line*>::iterator l;
    vector<Crack*>::iterator c;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l)->intersects(toCheck, intersect))
        {
            return true;
        }
        for (c = (*l)->cracks.begin(); c != (*l)->cracks.end(); ++c)
        {
            if ((*c)->lineIntersects(toCheck, intersect))
            {
                return true;
            }
        }
    }
    return false;
}
