#include "Crack.hpp"
#include "JDL.hpp"

#define _USE_MATH_DEFINES

#include <math.h>
#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;

void Crack::init(Shape *parentShape, Point startPoint, Line *startLine)
{
    this->shapeSplit = false;
    this->doDelete = false;
    this->intersectCrack = NULL;
    this->parentCrack = NULL;
    
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

Crack::Crack(string jsonString, Line *startLine)
{
    string intersectCrackStr, doDeleteStr, pointStr, shapeSplitStr;

    intersectCrackStr = grabJsonValue(jsonString, "intersectCrack");
    doDeleteStr = grabJsonValue(jsonString, "doDelete");
    pointStr = grabJsonValue(jsonString, "point");
    shapeSplitStr = grabJsonValue(jsonString, "shapeSplit");
    string linesString = grabJsonValue(jsonString, "lines");

    this->intersectCrack = NULL;
    if (intersectCrackStr != "false")
    {
        cerr << "intersectCrack was not Null. Cannot recreate!" << endl;
    }
    this->doDelete = (doDeleteStr == "true");
    this->startLine = startLine;
    this->parentShape = NULL;
    this->shapeSplit = (shapeSplitStr == "true");

    vector<string> jsonLines;
    vector<string>::iterator i;
    parseJsonList(&linesString, &jsonLines);
    for (i = jsonLines.begin(); i != jsonLines.end(); ++i)
    {
        lines.push_back(new Line(*i));
    }
}

Crack::Crack(const Crack &other)
{   
    this->intersectCrack = other.intersectCrack;
    this->parentCrack = other.parentCrack;
    this->doDelete = other.doDelete;
    this->startLine = other.startLine;
    this->parentShape = other.parentShape;
    this->shapeSplit = other.shapeSplit;
    
    //std::vector<Line*> lines;
    std::vector<Line*>::const_iterator l;
    for (l = other.lines.begin(); l != other.lines.end(); ++l)
    {
        this->lines.push_back(new Line(**l));
    }
}

void Crack::increaseOne(double force)
{
    //New Idea:
    //Create lines with random slopes starting from the given point until
    //One of those lines points towards the inside of the shape
    
    //Then, see if that line intersects with the edge of the shape
    //    (Or maybe even another crack)
    //If it does, split the shape apart.
    //    (If it intersects with multiple, choose the closest edge)

    int numIntersects = 0;
      
    vector<Point> intersectPoints;
    //vector<Line*> intersectLines;
    vector<Crack*> intersectCracks;
    Line fractureLine;
    double turnDegrees = 110;

    while (true)
    {
        double degree;
        
        //create a random slope of the line
        if (isPoint())
        {
            degree = JDL::randDouble(0, 360);
            //degree = rand() % 360;
        }
        else
        {
            double direction = lines.back()->getDirection();
            double modifier = JDL::randDouble(-turnDegrees/2, turnDegrees/2);
            //modifier = 45; //temporary debug lock
            degree = direction + modifier;
        }


        fractureLine = Line(lines.back()->point2, force, degree);
        JDL::setDrawColor(255, 0, 0);
        fractureLine.draw();
        JDL::setDrawColor(255,255,255);
        JDL::flush();

        bool badLine = false;
        /*
        vector<Line*>::iterator l;
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
        */
         //if it intersects itself, choose a new line
        Point selfIntersect;
        if (getTopParentCrack()->lineIntersectsIgnore(fractureLine,
                                                      fractureLine.point1))
        {
            //JDL::sleep(0.5);
            badLine = true;
        }


        if (badLine) 
        {
//            JDL::sleep(1);
            turnDegrees += 10;
            force *= 0.8;
            if (turnDegrees > 360)
            {
                turnDegrees = 360;
            }
            if (force < 1)
            {
                cerr << "Cannot increase this line" << endl; 
                return;
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
            //intersectLines.push_back(*i);
            intersectCracks.push_back(NULL);
        }
    }
    
    //Then check all the crack lines
    int numCrackIntersects;
    numCrackIntersects = parentShape->lineIntersectsCrack(fractureLine, 
                                                           &intersectCracks, 
                                                           &intersectPoints,
                                                          getTopParentCrack());

    numIntersects += numCrackIntersects;
    //Line *shapeLine = NULL;
    if (numIntersects > 0)
    {
        //getTopParentCrack()->shapeSplit = true;
        temp = this;
        if (intersectCracks[0])
        {
            //Note: this is a private variable
            intersectCrack = intersectCracks[0];
        }
        fractureLine.point2 = intersectPoints[0];

        vector<Point>::iterator p;
        for (p = intersectPoints.begin() + 1; p != intersectPoints.end(); ++p)
        {
            Line checkLine(fractureLine.point1, *p);
            if (checkLine.length() < fractureLine.length())
            {
                fractureLine.point2 = *p;
                intersectCrack = intersectCracks[p-intersectPoints.begin()];
            }
        }       
    }
    
    getTopParentCrack()->intersectCrack = intersectCrack;
    intersectCrack = NULL;

    addPoint(fractureLine.point2);
    //cout << "exiting increase()" << endl;
    parentShape->save("saves/After_Increase.txt");
    JDL::setDrawColor(0, 255, 0);
    fractureLine.draw();
    JDL::setDrawColor(255,255,255);
    JDL::flush();
}


void Crack::increase(double force)
{
    double decreaseFactor = 0.9;

    if (shapeSplit)
    {
        cerr << "Cannot increase crack that has already split the shape!"
             << endl;
        return;
    }

    if (isPoint())
    {
        increaseOne(force);
        return;
    }

    vector<Crack*> children;
    vector<Crack*>::iterator c;
    getChildren(&children);
    if (children.size())
    {
        //ok. If the crack has children, then we call increase on its children
        for (c = children.begin(); c != children.end(); ++c)
        {
            (*c)->increase(force * decreaseFactor);
        }
    }
    else
    {
        //the crack has no children.
        //first, we make some.
        //then, we call increase on those
        Crack *crack1 = new Crack(parentShape, lines.back()->point2, 
                                  lines.back());
        crack1->parentCrack = this;
        Crack *crack2 = new Crack(parentShape, lines.back()->point2,
                                  lines.back());
        crack2->parentCrack = this;

        lines.back()->cracks.push_back(crack1);
        lines.back()->cracks.back()->increase(force * decreaseFactor);
        lines.back()->cracks.push_back(crack2);
        lines.back()->cracks.back()->increase(force * decreaseFactor);
    }
}

void Crack::getChildren(vector<Crack*> *children)
{
    vector<Line*>::iterator l;
    vector<Crack*>::iterator c;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        for (c = (*l)->cracks.begin(); c != (*l)->cracks.end(); ++c)
        {
            children->push_back(*c);
        }
    }
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

bool Crack::isPoint()
{
    if (!lines.size()) return true;
    if (lines[0]->point1 == lines[0]->point2)
    {
        if (lines.size() != 1)
        {
            cerr << "The first line in the crack is a point!" << endl;
        }
        return true;
    }
    return false;
}

void Crack::addPoint(Point toAdd)
{
    //cout << "size: " << lines.size() << endl;
    if (!lines.size())
    {
        lines.push_back(new Line(toAdd, toAdd));
    }
    else if (isPoint())
    {
        lines[0]->point2 = toAdd;
    }
    else
    {
        lines.push_back(new Line(lines.back()->point2, toAdd));
    }
    //lines.back()->index = lines.size()-1;
}


void Crack::clearLines()
{
    lines.clear();
}

//starting from this crack, go up the chain an grab all the lines
void Crack::convertToLinesDelete(vector<Line*> *resultVec, Crack *toDelete)
{
    //first step: throw all lines of this crack into the vector.
    //TODO: efficiency. I know pushing front to a vector is bad.

    vector<Line*>::reverse_iterator rl;
    for (rl = lines.rbegin(); rl != lines.rend(); ++rl)
    {
        cout << *rl;
        (*rl)->deleteCrack(toDelete);
        resultVec->insert(resultVec->begin(), *rl);
    }

    //second step: go up one level.
    //push all lines of parent, except not this crack.
    if (parentCrack)
    {
        parentCrack->convertToLinesDelete(resultVec, this);
    }
}

void Crack::getSplitLines(vector<Line*> *splitLines)
{
#ifdef IM_TIRED
    //TODO: We're going to ignore crack-crack intersections for now
    
    vector<Line*>::iterator l;
    
    //first, get the split lines from the intersectCrack
    vector<Line*> intersectCrackLines;
    vector<Line*> newCrack;
    bool afterIntersect = false;
    if (intersectCrack)
    {
        for (l = intersectCrack->lines.begin();
             l != intersectCrack->lines.end(); ++l)
        {
            if (!afterIntersect)
            {
                intersectCrackLines.push_back(*l);
            }
            else
            {
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
    if (!temp) cout << "Welp..." << endl;
    temp->convertToLinesDelete(splitLines, NULL);
    cout << "HERE" << endl;
/*   
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        splitLines->push_back(*l);
    }
*/
   
    
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
        intersectCrack->startLine = lines.back();
        //intersectCrack->shapeSplit = false;
        lines.back()->cracks.push_back(intersectCrack);

        /*S*DF*SDF**/
        //Step 2: add newCrack to lines.back()
        //lines.back()->cracks.push_back(new Crack(parentShape, newCrack, 
        //lines.back()));

        //Step 3: Mark intersectCrack for deletion
        //intersectCrack->doDelete = true;
/*
        JDL::setDrawColor(0, 128, 255);
        drawLines(*splitLines, 1);
        JDL::setDrawColor(255,255,255);
*/
        //JDL::sleep(5);

    }
#endif
}

bool Crack::isShapeSplit()
{
    return false;
    //return shapeSplit;
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

Line *Crack::getFirstLine()
{
    return lines[0];
}

bool Crack::lineIntersectsIgnore(const Line &toCheck, Point toIgnore)
{
    vector<Line*>::iterator l;
    vector<Crack*>::iterator c;
    Point intersect;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l)->intersects(toCheck, &intersect))
        {
            if (intersect != toIgnore)
            {
                return true;
            }
        }
        for (c = (*l)->cracks.begin(); c != (*l)->cracks.end(); ++c)
        {
            if ((*c)->lineIntersectsIgnore(toCheck, toIgnore))
            {
                return true;
            }
        }
    }
    return false;
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

//NOTE: sanityCheck MODIFIES to make sane!
bool Crack::sanityCheck(Shape *parentShape, Line *startLine)
{
    bool toReturn = true;
    if (parentShape != this->parentShape)
    {
        toReturn = false;
        cerr << "Insanity! Wrong parentShape found. Fixing..." << endl;
        this->parentShape = parentShape;
    }
    if (startLine != this->startLine)
    {
        toReturn = false;
        cerr << "Insanity! Wrong startLine found. Fixing..." << endl;
        this->startLine = startLine;
    }
    return toReturn;
}

string Crack::generateJSON()
{
    string toReturn;
    vector<Line*>::iterator l;
    stringstream streamy;
    
    //note: parentShape and startLine NOT tracked
    streamy << "{";
    streamy <<"\"intersectCrack\":" << ((intersectCrack) ? "true" : "false");
    streamy << ",\"parentShape\":" << ((parentShape) ? "true" : "false"); //debug only
    streamy << ",\"doDelete\":" << ((doDelete) ? "true" : "false");
    streamy << ",\"shapeSplit\":" << ((shapeSplit) ? "true" : "false");
    streamy << ",\"lines\":";


    toReturn = streamy.str();

    toReturn += "[";
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        toReturn += (*l)->generateJSON();
        if (l + 1 != lines.end())
        {
            toReturn += ",";
        }
    }
    toReturn += "]";
    toReturn += "}";
    return toReturn;
}

Crack* Crack::getTopParentCrack()
{
    if (!parentCrack)
    {
        return this;
    }
    else
    {
        return parentCrack->getTopParentCrack();
    }
/*
    if (parentShape->containsLine(startLine))
    {
        return this;
    }
    else
    {
        return startLine->getParentCrack();
    }
*/
}
