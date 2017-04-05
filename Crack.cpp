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
    //this->intersectCrack = NULL;
    this->parentCrack = NULL;
    //this->temp = NULL;
    
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

    //this->intersectCrack = NULL;
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
    //this->intersectCrack = other.intersectCrack;
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

#if 0
void Crack::increaseOld(double force)
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
            //intersectLines.push_back(*i);
            intersectCracks.push_back(NULL);
        }
    }
    
    //Then check all the crack lines
    int numCrackIntersects;
    numCrackIntersects = parentShape->lineIntersectsCrack(fractureLine, 
                                                           &intersectCracks, 
                                                           &intersectPoints,
                                                           this);
    /*
    int counter;
    for (counter = 0; counter < numCrackIntersects; counter++)
    {
        intersectLines.push_Back(NULL);
    }
    */

    numIntersects += numCrackIntersects;
    //Line *shapeLine = NULL;
    if (numIntersects > 0)
    {
        shapeSplit = true;
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

        //
        
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
    //cout << "exiting increase()" << endl;
    //parentShape->save("saves/After_Increase.txt");
    JDL::setDrawColor(0, 255, 0);
    fractureLine.draw();
    JDL::setDrawColor(255,255,255);
    JDL::flush();
}
#endif

//This is the currently used Increase
void Crack::increaseOne(double force)
{
    //New Idea:
    //Create lines with random slopes starting from the given point until
    //One of those lines points towards the inside of the shape
    
    //Then, see if that line intersects with the edge of the shape
    //    (Or maybe even another crack)
    //If it does, split the shape apart.
    //    (If it intersects with multiple, choose the closest edge)

    //int numIntersects = 0;
      
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

    //New Code:
    //first, check all the shape lines
    Point intersectPoint;
    bool intersect = parentShape->
        lineIntersectsBorderCrackNearest(fractureLine, &intersectPoint);

    if (intersect)
    {
        this->shapeSplit = true; //will get set to false if no intersect
        fractureLine.point2 = intersectPoint;
    }

#if 0

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
        shapeSplit = true;
        //getTopParentCrack()->shapeSplit = true;
        //getTopParentCrack()->temp = this;
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
#endif
    //getTopParentCrack()->intersectCrack = intersectCrack;
    //intersectCrack = NULL;
    parentShape->save("saves/After_Increase.txt");
    addPoint(fractureLine.point2);
    //cout << "exiting increase()" << endl;
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

        //finally, save:
        //parentShape->save("saves/After_Increase.txt");
    }
}

void Crack::getChildren(vector<Crack*> *children)
{
    vector<Line*>::const_iterator l;
    vector<Crack*>::const_iterator c;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        for (c = (*l)->cracks.begin(); c != (*l)->cracks.end(); ++c)
        {
            children->push_back(*c);
        }
    }
}

void Crack::getGrandestChildren(vector <Crack*> *grandChildren)
{
    vector<Crack*> directChildren;
    getChildren(&directChildren);
    
    vector<Crack*>::const_iterator c;
    for (c = directChildren.begin(); c != directChildren.end(); ++c)
    {
        (*c)->getGrandestChildren(grandChildren);
    }

    if (directChildren.size() == 0)
    {
        grandChildren->push_back(this);
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
        JDL::setDrawColor(128, 128, 128);
        (*i)->draw(); //oo this looks like a possible stack overflow :)
        //(Look at Line's draw. it calls this draw).
        JDL::setDrawColor(255, 255, 255);
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

//starting from this crack, go up the chain and grab all the lines
void Crack::convertToLinesDelete(vector<Line*> *resultVec, Crack *toDelete)
{
    //first step: throw all lines of this crack into the vector.
    //TODO: efficiency. I know pushing front to a vector is bad.

    cout << "sizeof lines" << lines.size() << endl;

    vector<Line*>::reverse_iterator rl;
    vector<Crack*>::iterator c;
    for (rl = lines.rbegin(); rl != lines.rend(); ++rl)
    {
        //cout << *rl;
        (*rl)->deleteCrack(toDelete);
        
        //we have to tell the child cracks that they no longer have a parent.
        for (c = (*rl)->cracks.begin(); c != (*rl)->cracks.end(); ++c)
        {
            (*c)->parentCrack = NULL;
        }

        resultVec->insert(resultVec->begin(), *rl);
    }

    //second step: go up one level.
    //push all lines of parent, except not this crack.
    if (parentCrack)
    {
        parentCrack->convertToLinesDelete(resultVec, this);
    }
    else
    {
        startLine->deleteCrack(this);
    }
}

//This crack becomes a fragment of what it used to be.
//the new crack is returned.
//note: Do not search child cracks for the split.
//DO NOT add the new crack to the end of this crack (or anything!)
//however, set the startLine to be the proper line
Crack *Crack::splitOffAt(Point splitPoint)
{
    Crack *newCrack = NULL;
    vector<Line*>::iterator l;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l)->on(splitPoint))
        {
            Line *newLine = new Line();
            (*l)->split(splitPoint, newLine);
            newCrack = new Crack(this->parentShape, splitPoint, *l);
            //(*l)->cracks.push_back(newCrack);
            newCrack->lines.assign(l+1, lines.end());
            newCrack->lines.insert(newCrack->lines.begin(), newLine);
            l = lines.erase(l+1, lines.end());
            break;
        }
    }
    return newCrack;
}

void Crack::getSplitLines(vector<Line*> *splitLines)
{
    Line *backLine = lines.back();

    //delete a crack from this line.
    //add all the lines in this crack.
    convertToLinesDelete(splitLines, NULL);
    //convertToLinesRemove?
    
    Crack *intersectCrack = NULL;
    Point intersectPoint;
    if (parentShape->lineIntersectsCrackNearest(*backLine, &intersectPoint,
                                                 &intersectCrack, this))
    {
        if (intersectPoint != backLine->point2)
        {
            cerr << "Well, that's interesting..." << endl;
        }
        //step 1: split the crack that we intersected.
        Crack *newCrack = intersectCrack->splitOffAt(backLine->point2);
        if (!newCrack)
        {
            cerr << "Error: This crack does not end on intersectCrack" << endl;
            return;
        }

        newCrack->startLine = lines.back();
        lines.back()->cracks.push_back(newCrack);
        newCrack->parentCrack = this;

        vector<Line*> intersectLines;

        //Step 2: convert the intersected Crack to splitLines
        intersectCrack->convertToLinesDelete(&intersectLines, NULL);

        //Step 3: Flip the intersectedCrackLines.
        //(Flip both their position in the vector and their point1/point2)
        vector<Line*>::reverse_iterator rl;
        for (rl = intersectLines.rbegin(); rl != intersectLines.rend(); ++rl)
        {
            (*rl)->switchPoints();
            splitLines->push_back(*rl);
        }
    }

#if 1    
    JDL::setDrawColor(0, 255, 255);
    drawLines(*splitLines, 0);
    JDL::setDrawColor(255, 255, 255);
    JDL::flush();
    vector<Shape*> fakeToDraw;
    Shape temp(*splitLines, &fakeToDraw);
    temp.save("saves/splitLines.txt");
    //char garbage;
    //cin >> garbage;
#endif
}

#if 0
void Crack::getSplitLines(vector<Line*> *splitLines)
{
    //TODO: We're going to ignore crack-crack intersections for now
    
    vector<Line*>::iterator l;
    
    //first, get the split lines from the intersectCrack
    vector<Line*> intersectCrackLines;
    vector<Line*> newCrack;
    bool afterIntersect = false;
    if (intersectCrack)
    {
        cout << "IntersectCrack is not NULL! This is a problem for now" << endl;
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
    cout << "temp is " << temp << endl;
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
}
#endif

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

bool Crack::lineIntersects(const Line &toCheck, Point *intersect, 
                           Crack **intersectCrack)
{
    vector<Line*>::iterator l;
    vector<Crack*>::iterator c;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if ((*l)->intersects(toCheck, intersect))
        {
            *intersectCrack = this;
            return true;
        }
        for (c = (*l)->cracks.begin(); c != (*l)->cracks.end(); ++c)
        {
            if ((*c)->lineIntersects(toCheck, intersect, intersectCrack))
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
    vector<Line*>::iterator l;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        (*l)->sanityCheck(parentShape);
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
    //streamy <<"\"intersectCrack\":" << ((intersectCrack) ? "true" : "false");
    streamy << "\"parentShape\":" << ((parentShape) ? "true" : "false"); //debug only
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

Crack* Crack::addChild()
{
    Crack *toAdd = new Crack(parentShape, lines.back()->point2, 
                             lines.back());
    toAdd->parentCrack = this;
    lines.back()->cracks.push_back(toAdd);
    return toAdd;
}
