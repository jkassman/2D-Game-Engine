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

Crack::Crack(const Crack &other) : Line(other)
{   
    this->parentCrack = other.parentCrack;
    this->shapeSplit = other.shapeSplit;
    
    vector<Crack*>::const_iterator c;
    for (c = other.childCracks.begin(); c != other.childCracks.end(); ++c)
    {
        childCracks.push_back(new Crack(**c));
    }
}


#define EXPAND_DEBUG
//expands from a point to a line.
void Crack::expand(double force, Shape *parentShape)
{
    //New Idea:
    //Create lines with random slopes starting from the given point until
    //One of those lines points towards the inside of the shape
    
    //Then, see if that line intersects with the edge of the shape
    //    (Or another crack)
    //If it does, set shapeSplit to true. This will evnetually split the shape.
    //    (If it intersects with multiple, choose the closest edge)

    if (!isPoint()) 
    {
        cerr << "Cannot expand anything other than a point." << endl;
        return;
    }

    //vector<Point> intersectPoints;
    //vector<Crack*> intersectCracks;
    Line fractureLine;
    double turnDegrees = 110;

    while (true)
    {
        double degree;
        
        //create a random slope of the line
        if (parentCrack == NULL)
        {
            degree = JDL::randDouble(0, 360);
        }
        else
        {
            double direction = parentCrack->getDirection();
            double modifier = JDL::randDouble(-turnDegrees/2, turnDegrees/2);
            //modifier = 45; //temporary debug lock
            degree = direction + modifier;
        }

        fractureLine = Line(point1, force, degree);
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
        /*
        if (parentShape->lineOnBorder(fractureLine))
        {
            JDL::setDrawColor(255, 0, 255);
            fractureLine.draw();
            JDL::setDrawColor(255,255,255);
            JDL::flush();
            continue;
        }
        */

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

void Crack::translate(double distance, double degrees)
{
    ((Line*) this)->translate(distance, degrees);

    vector<Crack*>::iterator c;
    for (c = childCracks.begin(); c != childCracks.end(); ++c)
    {
        (*c)->translate(distance, degrees);
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

/*
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
*/

 /*
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
 */
/*
//starting from this crack, go up the chain and grab all the lines
void Crack::convertToLinesDelete(vector<Line*> *resultVec, Crack *toDelete)
{
    //first step: throw all lines of this crack into the vector.
    //TODO: efficiency. I know pushing front to a vector is bad.

    vector<Crack*>::iterator c;
    deleteCrack(toDelete);
    
    //we have to tell the child cracks that they no longer have a parent.
    for (c = childCracks.begin(); c != childCracks.end(); ++c)
    {
        (*c)->parentCrack = NULL;
    }

    //TODO: Does this work!? Efficiency!? Memory leak!?
    resultVec->insert(resultVec->begin(), this);
    
    //second step: go up one level.
    //push all lines of parent, except not this crack.
    if (parentCrack)
    {
        parentCrack->convertToLinesDelete(resultVec, this);
    }
    else
    {
        parentShape->deleteCrack(this);
    }
}
*/
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
        for (c = parentCrack->childCracks.begin();
             c != parentCrack->childCracks.end(); ++c)
        {
            if ((*c) == this)
            {
                parentCrack->childCracks.erase(c);
                break;
            }
        }
        if (c == childCracks.end())
        {
            cerr << "ERROR: Failed to delete this crack!" << endl;
        }
        
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

/*
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
*/

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
        cout << "GOING HERE!!!" << endl;
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
    
#if 1
    vector<Crack*>::iterator c;
    sort(splitCracks->begin(), splitCracks->end());
    Crack *previous = NULL;
    for (c = splitCracks->begin(); c != splitCracks->end(); ++c)
    {
        if (previous == *c)
        {
            cout << "Aha! Found a duplicate!" << endl;
        }
        previous = *c;
    }
    
#endif
#if 1    
    JDL::setDrawColor(0, 255, 255);
    drawLines(*splitLines, 0);
    JDL::setDrawColor(255, 255, 255);
    JDL::flush();
#ifdef RESEARCH_SAVE_ONE
    vector<Shape*> fakeToDraw;
    Shape temp(*splitLines, &fakeToDraw);
    temp.save("saves/splitLines.txt");
#endif
    //char garbage;
    //cin >> garbage;
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

//NOTE: sanityCheck MODIFIES to make sane!
/*
bool Crack::sanityCheck(Shape *parentLine *startLine)
{
    bool toReturn = true;
    if (parentShape != this->parentShape)
    {
        toReturn = false;
        cerr << "Insanity! Wrong parentShape found. Fixing..." << endl;
        this->parentShape = parentShape;
    }

    if (((Line*) this)->sanityCheck(parentShape) == false)
    {
        toReturn = false;
    }
    
    return toReturn;
}
*/
/*
string Crack::generateJSON()
{
    string toReturn;
    vector<Line*>::iterator l;
    stringstream streamy;
    
    //note: parentShape  NOT tracked
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
*/

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
