#include "Crack.hpp"
#include "JDL.hpp"
#include "jacobJSON.hpp"

#define _USE_MATH_DEFINES

#include <math.h>
#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;

void Crack::setDefaults()
{
    this->shapeSplit = false;
    this->parentCrack = NULL;
}

Crack::Crack(Point startPoint) : Line(startPoint)
{
    setDefaults();
}

Crack::Crack(const Line &otherLine) : Line(otherLine)
{
    setDefaults();
}

Crack::Crack(string jsonString, string jsonLine, 
             Crack *parentCrack) : Line(jsonLine)
{
    this->parentCrack = parentCrack;
    string shapeSplitString = grabJsonValue(jsonString, "shapeSplit");
    this->shapeSplit = (shapeSplitString == "true");
    
    string childCracksJson = grabJsonValue(jsonString, "childCracks");
    vector<string> childCracksStrings;
    parseJsonList(&childCracksJson, &childCracksStrings);
    vector<string>::iterator i;
    for (i = childCracksStrings.begin(); i != childCracksStrings.end(); ++i)
    {
        string childLine = grabJsonValue(*i, "line");
        childCracks.push_back(new Crack(*i, childLine, this));
    }
}

//if deep is false, shallow copy
Crack::Crack(const Crack &other, bool deep, Crack* parent) : Line(other)
{   
    if (deep)
    {
        this->parentCrack = parent;
    }
    else
    {
        this->parentCrack = other.parentCrack;
    }
    this->shapeSplit = other.shapeSplit;
    
    vector<Crack*>::const_iterator c;
    for (c = other.childCracks.begin(); c != other.childCracks.end(); ++c)
    {
        if (deep)
        {
            childCracks.push_back(new Crack(**c, true, this));
        }
        else
        {
            childCracks.push_back(*c);
        }
    }
}


#define _P_DEBUG
//expands from a point to a line.
void Crack::expand(double force, Shape *parentShape)
{
    //Create lines with random slopes starting from the given Crack
    //Until one of those lines points towards the inside of the shape
    
    //Then, see if that line intersects with the edge of the shape
    //    (Or another crack)
    //If it does, set shapeSplit to true. This will eventually split the shape.
    //    (If it intersects with multiple, choose the closest edge)

    if (!isPoint()) 
    {
        cerr << "Cannot expand anything other than a point." << endl;
        return;
    }

    Line fractureLine;
    double turnRadians = 110 * M_PI/180;

    while (true)
    {
        double radian;
        
        //create a random slope of the line
        if (parentCrack == NULL)
        {
            radian = JDL::randDouble(0, 2 * M_PI);
        }
        else
        {
            double direction = parentCrack->getDirection();
            double modifier = JDL::randDouble(-turnRadians/2, turnRadians/2);
            //modifier = 45; //temporary debug lock
            radian = direction + modifier;
        }

        fractureLine = Line(point1, force, radian);
        fractureLine.ignorePoint = 1;
#ifdef EXPAND_DEBUG
        JDL::setDrawColor(255, 0, 0);
        fractureLine.draw();
        JDL::setDrawColor(255,255,255);
        JDL::flush();
#endif

        //if it intersects itself, choose a new line
        bool badLine = false;
        Point selfIntersect;
        if (getTopParentCrack()->lineIntersects(fractureLine))
        {
            badLine = true;
        }


        if (badLine) 
        {
            turnRadians += 10 * M_PI/180;
            force *= 0.8;
            if (turnRadians > 2 * M_PI)
            {
                turnRadians = 2 * M_PI;
            }
            if (force < 1)
            {
                cerr << "Cannot increase this line" << endl; 
                return;
            }
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
    }
    //Now we know the crack is in the right direction.
    //But we need to count how many shape lines it intersects
    //(if it intersects no lines, we're done)

    Point intersectPoint;
    bool intersect = parentShape->
        lineIntersectsBorderCrackNearest(fractureLine, &intersectPoint);

    if (intersect)
    {
        this->shapeSplit = true;
        fractureLine.point2 = intersectPoint;
    }

    point2 = fractureLine.point2;
#ifdef RESEARCH_SAVE_ONE
    parentShape->save("saves/After_Increase.txt");
#endif
#ifdef RESEARCH_SAVE_STORY
    cout << "SAVING" << endl;
    stringstream streamy;
    streamy << "story/save" << parentShape->saveNum << ".txt";
    saveShapes(streamy.str(), parentShape->toDraw);
    parentShape->saveNum++;
#endif
#ifdef EXPAND_DEBUG
    JDL::setDrawColor(0, 255, 0);
    fractureLine.draw();
    JDL::setDrawColor(255,255,255);
    JDL::flush();
#endif
}

void Crack::getGrandestChildren(vector <Crack*> *grandChildren)
{
    vector<Crack*>::const_iterator c;
    for (c = childCracks.begin(); c != childCracks.end(); ++c)
    {
        (*c)->getGrandestChildren(grandChildren);
    }

    if (childCracks.size() == 0)
    {
        grandChildren->push_back(this);
    }
}

void Crack::getAllExceptGrandestChildren(vector<Crack*> *parents)
{
    if (childCracks.size() == 0) return;
    
    parents->push_back(this);
    vector<Crack*>::iterator c;
    for (c = childCracks.begin(); c != childCracks.end(); ++c)
    {
        (*c)->getAllExceptGrandestChildren(parents);
    }
}

void Crack::scale(double factor)
{
    ((Line*) this)->scale(factor);
    vector<Crack*>::iterator c;
    for (c = childCracks.begin(); c != childCracks.end(); ++c)
    {
        (*c)->scale(factor);
    }
}

void Crack::translate(double distance, double radians)
{
    ((Line*) this)->translate(distance, radians);

    vector<Crack*>::iterator c;
    for (c = childCracks.begin(); c != childCracks.end(); ++c)
    {
        (*c)->translate(distance, radians);
    }
}

void Crack::rotateAbout(double theta, Point about)
{
    ((Line*) this)->rotateAbout(theta, about);
    vector<Crack*>::iterator c;
    for (c = childCracks.begin(); c != childCracks.end(); ++c)
    {
        (*c)->rotateAbout(theta, about);
    }
}

void Crack::draw() const
{
    if (!parentCrack) JDL::setDrawColor(128, 128, 128);
    
    ((Line*) this)->draw();

    vector<Crack*>::const_iterator c;
    for (c = childCracks.begin(); c != childCracks.end(); ++c)
    {
        (*c)->draw();
    }
    
    if (!parentCrack) JDL::setDrawColor(255, 255, 255);
}

Point Crack::startPoint()
{
    return point1;
}

void Crack::deleteChild(Crack *toDelete)
{
    vector<Crack*>::iterator c;
    for (c = childCracks.begin(); c != childCracks.end(); ++c)
    {
        if ((*c) == toDelete)
        {
            childCracks.erase(c);
            return;
        }
    }
    if (c == childCracks.end())
    {
        cerr << "ERROR: Failed to delete this crack!" << endl;
    }
}

 //#define CONVERT_TO_LINES_AND_CRACKS_DEBUG
//NOTE: This is literally suicide. It deletes itself at the end!
void Crack::convertToLinesAndCracks(vector<Line*> *linesVec, 
                                    vector<Crack*> *cracksVec,
                                    Shape *parentShape)
{
#ifdef CONVERT_TO_LINES_AND_CRACKS_DEBUG
    JDL::setDrawColor(0, 0, 255);
    draw();
    JDL::setDrawColor(45, 45, 45);
    JDL::sleep(1);
#endif
    //step 1: grab all the cracks from this line and throw them into a vector.
    //also inform them that they are now orphans
    vector<Crack*>::iterator c;
    for (c = childCracks.begin(); c != childCracks.end(); ++c)
    {
        (*c)->parentCrack = NULL;
        cracksVec->push_back(*c);
    }
    
    //step 2: convert this crack to a line:
    linesVec->insert(linesVec->begin(), (new Line(point1, point2)));

    //steps 3-4: only occur if parentCrack exists
    if (parentCrack) 
    {
        //step 3: delete this crack from its parent
        parentCrack->deleteChild(this);
        
        //step 4: recursion!
        parentCrack->convertToLinesAndCracks(linesVec, cracksVec, parentShape);
    }
    else
    {
        parentShape->deleteCrack(this);
    }
    //step 5: kill self
    childCracks.clear();
    delete this;
}

 //Set returned crack's parent to be "oldCrack"; e.g. this
Crack *Crack::split(Point splitPoint)
{
    Line newLine;
    ((Line*) this)->split(splitPoint, &newLine);
    Crack *newCrack = new Crack(newLine);
    
    //remove all children from this and give them to newCrack
    vector<Crack*>::iterator c = childCracks.begin();
    while (c != childCracks.end())
    {
        newCrack->childCracks.push_back(*c);
        (*c)->parentCrack = newCrack;
        c = childCracks.erase(c);
    }
    
    //set newCrack's parent to be this
    newCrack->parentCrack = this;
    return newCrack;
}

void Crack::getSplitLinesAndCracks(vector<Line*> *splitLines, 
                                   vector<Crack*> *splitCracks,
                                   Shape *parentShape)
{ 
    Crack *intersectCrack = NULL;
    Point intersectPoint;
    this->ignorePoint = 1;
    if (parentShape->lineIntersectsCrackNearest(*this, &intersectPoint,
                                                 &intersectCrack, this))
    {
        if (intersectPoint != point2)
        {
            cerr << "Well, that's interesting..." << endl;
        }
        //step 1: split the crack that we intersected.
        Crack *newCrack = intersectCrack->split(point2);
        newCrack->parentCrack = NULL;
        if (!newCrack)
        {
            cerr << "Error: This crack does not end on intersectCrack" << endl;
            return;
        }
        splitCracks->push_back(newCrack);

        //Step 2: convert the intersected Crack to splitLines
        intersectCrack->convertToLinesAndCracks(splitLines, splitCracks, 
                                                parentShape);
    }
    
    //(Flip both the position in the vector and their point1/point2)
    vector<Line*> selfLines;
    convertToLinesAndCracks(&selfLines, splitCracks, parentShape);
    vector<Line*>::reverse_iterator rl;
    for (rl = selfLines.rbegin(); rl != selfLines.rend(); ++rl)
    {
        (*rl)->switchPoints();
        splitLines->push_back(*rl);
    }
    

#ifdef RESEARCH_SAVE_ONE
    vector<Shape*> fakeToDraw;
    Shape temp(*splitLines, &fakeToDraw);
    temp.save("saves/splitLines.txt");
#endif
}

bool Crack::isShapeSplit()
{
    return shapeSplit;
}

bool Crack::lineIntersects(const Line &toCheck)
{
    Point dummyPoint;
    Crack *dummyCrack;
    return lineIntersects(toCheck, &dummyPoint, &dummyCrack);
}

//returns childmost intersected crack
bool Crack::lineIntersects(const Line &toCheck, Point *intersect, 
                           Crack **intersectCrack)
{
    vector<Crack*>::iterator c;
    if (intersects(toCheck, intersect))
    {
        *intersectCrack = this;
        return true;
    }
    for (c = childCracks.begin(); c != childCracks.end(); ++c)
    {
        if ((*c)->lineIntersects(toCheck, intersect, intersectCrack))
        {
            return true;
        }
    }
    return false;
}

string Crack::generateJSON() const
{
    string toReturn;
    stringstream streamy;
    
    //note: parentShape  NOT tracked
    streamy << "{";
    streamy << "\"line\":" << ((Line*)this)->generateJSON();
    streamy << ",\"parentCrack\":" << ((parentCrack) ? "true" : "false");
    streamy << ",\"shapeSplit\":" << ((shapeSplit) ? "true" : "false");
    streamy << ",\"childCracks\":[";
    
    vector<Crack*>::const_iterator c;
    for (c = childCracks.begin(); c != childCracks.end(); ++c)
    {
        streamy << (*c)->generateJSON();
        if (c + 1 != childCracks.end())
        {
            streamy << ",";
        }
    }
    streamy << "]}";
    return streamy.str();
}

Crack *Crack::getParentCrack()
{
    return parentCrack;
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
}

Crack* Crack::addChild()
{
    Crack *toAdd = new Crack(point2);
    toAdd->parentCrack = this;
    childCracks.push_back(toAdd);
    return toAdd;
}
