#include "Shape.hpp"
#include "JDL.hpp"

#define _USE_MATH_DEFINES

#include <iostream>
#include <algorithm>
#include <math.h>

using namespace std;
Shape::Shape(vector<Point> givenPoints, vector<Shape*> *toDraw)
{
    this->speed = 0;
    this->direction = 0;
    this->toDraw = toDraw;
    vector<Point>::iterator i;
    vector<Point>::iterator next;
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
}

Shape::Shape(vector<Line*> &givenLines, vector<Shape*> *toDraw)
{
    this->speed = 0;
    this->direction = 0;
    this->toDraw = toDraw;
    this->lines = givenLines;
}

bool Shape::inside(Point toTest)
{
    return false;
}

void Shape::move(double distance, double degrees)
{
    vector<Line*>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        (*i)->move(distance, degrees);
    }
}

void Shape::move()
{
    vector<Line*>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        (*i)->move(speed, direction);
    }
}

void Shape::setSpeed(double speed, double degrees)
{
    this->speed = speed;
    this->direction = degrees;
}

void Shape::draw()
{
    vector<Line*>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        (*i)->draw();
    }
}

//returns how many cracks were affected
int Shape::fractureAt(Point clickPoint)
{
    vector<Crack*> impactedCracks;
    vector<Line*>::iterator i;
    double force = 100;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        Point result;
        if ((*i)->on(clickPoint, 10, &result))
        {
            //JDL::circle(result.x, result.y, 8);            
            (*i)->getImpactedCracks(result, this, force, &impactedCracks);
            break;
        }
    }
    
    //now update the cracks that we have gathered.
    vector<Crack*>::iterator c;
    vector<Line*> splitLines;
    cout << impactedCracks.size() << endl;
    for (c = impactedCracks.begin(); c != impactedCracks.end(); ++c)
    {
        //eventually, modify force here based on size of impactedCracks
        (*c)->increase(force);
        if ((*c)->isShapeSplit())
        {
            (*c)->getSplitLines(&splitLines);
            (*c)->clearLines(); //so the destructor doesn't erase the lines
            (*c)->getStartLine()->deleteCrack(*c);
            split(splitLines);
        }
    }
    return impactedCracks.size();
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
    lines.back()->index = lines.size()-1;
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
void Shape::split(vector<Line*> &splitLines)
{
    //new new plan:
    //split the shape by lines, create two new shapes from them.
    //split startLine, split endLine, then throw the lines into shapes.

    //temporary fix:
    Line* startLine = splitLines.front();
    splitLines.erase(splitLines.begin());
    Line* endLine = splitLines.back();
    splitLines.pop_back();


    //endLine splits intersectPoint->point2.
    vector<Line*> lines1, lines2;

    
    Point crackStartPoint, crackEndPoint;
    crackStartPoint = splitLines[0]->point1;
    crackEndPoint = splitLines.back()->point2;
#ifdef OLD_CODE
    vector<Line*>::iterator l;
    int start1Index, start2Index, end1Index, end2Index;
    start1Index = startLine->index;
    end1Index = endLine->index;
    start2Index = endLine->index;
    end2Index = startLine->index;
    
    Line *start1, *start2;
    start1 = new Line();
    start2 = new Line();
    
    bool sameLineSplit = (startLine == endLine);

    if (sameLineSplit)
    {
        cout << "Same Line!" << endl;
        Point farPoint, closePoint; //far or close to startLine->point1
        Line firstSplit(startLine->point1, crackStartPoint);
        Line secondSplit(startLine->point1, crackEndPoint);
        if (firstSplit.length() > secondSplit.length())
        {
            farPoint = crackStartPoint;
            closePoint = crackEndPoint;
        }
        else
        {
            farPoint = crackEndPoint;
            closePoint = crackStartPoint;
        }
     
        //we want endLine (end1) to be point1->closePoint
        //and start1 to be farPoint->point2
        
        //start2 and end2 are the same line, closePoint->farPoint
        
        endLine->split(closePoint, start2);
        //endLine: point1->closePoint
        //start2: closePoint->point2.

        start2->split(farPoint, start1);
        //start2: closePoint->farPoint
        //start1: farPoint->point2
    }
    else
    {
        //start1: crackStartPoint->startLine.Point2
        //end1: endLine.point1->crackEndPoint
        //start2: crackEndPoint->endLine.point2
        //end2: startLine.point1->crackStartPoint
        //end1 and end2 should remain as belonging to this shape;
        //start1 and start2 will just be inserted into the vector.
 
        Line *end1, *end2;
        end2 = startLine;
        end1 = endLine;
        end2->split(crackStartPoint, start1);
        end1->split(crackEndPoint, start2);
    }

    start1->index = 0;
    start2->index = 0;

    lines1.push_back(start1);
    cout << "start1Index: " << start1Index << "end1Index" << end1Index << endl;
    grabShapeLines(start1Index+1, end1Index, &lines1);
    
    lines2.push_back(start2);
    if (!sameLineSplit)
    {
        grabShapeLines(start2Index+1, end2Index, &lines2);
    }
#else
    vector<Line*>::iterator l;
    if (startLine == endLine)
    {
        Point farPoint, closePoint; //far or close to startLine->point1
        Line firstSplit(startLine->point1, crackStartPoint);
        Line secondSplit(startLine->point1, crackEndPoint);
        if (firstSplit.length() > secondSplit.length())
        {
            farPoint = crackStartPoint;
            closePoint = crackEndPoint;
            //let's reverse the splitLines, to be safe.
            for (l = splitLines.begin(); l != splitLines.end(); ++l)
            {
                (*l)->switchPoints();
            }
        }
        else
        {
            farPoint = crackEndPoint;
            closePoint = crackStartPoint;
        }
        crackStartPoint = closePoint;
        crackEndPoint = farPoint;
    }
    
    //split the lines at the crack.
    vector<Line*>::iterator next;
    for (l = lines.begin(); l != lines.end();)
    {
        next = l + 1;
        if (*l == startLine)
        {
            Line *nextLine = new Line();
            (*l)->split(crackStartPoint, nextLine);
            next = lines.insert(l+1, nextLine);
        }
        if (*l == endLine)
        {
            Line *nextLine = new Line();
            (*l)->split(crackEndPoint, nextLine);
            next = lines.insert(l+1, nextLine);
        }
        l = next;
    }
    
    vector<Line*>::iterator start1It, end1It, start2It, end2It;
    //discover the split lines
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        if (*l == startLine)
        {
            end2It = l;
            start1It = l + 1;
        }
        //note: Important that it's not else if for same line case
        if (*l == endLine)
        {
            end1It = l;
            start2It = l + 1;
        }
    }
    
    grabShapeLines(start1It, end1It, &lines1);
    grabShapeLines(start2It, end2It, &lines2);
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
    this->lines.clear();
    this->lines.assign(lines2.begin(), lines2.end());
    
    if (!sanityCheck())
    {
        debugDraw();
    }


    /* Create a new shape (using lines1) */
    Shape *newShape = new Shape(lines1, this->toDraw);
    if (!sanityCheck())
    {
        debugDraw();
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

    //Set all the cracks in the new shape to have their parent be newShape
    for (l = newShape->lines.begin(); l != newShape->lines.end(); ++l)
    {
        (*l)->setCrackParents(newShape);
    }
                     

    //TODO: Delete splitlines here! We made two copies

    /*
    //DEBUG DRAW:
    char garbage;
    vector<Line*>::iterator debug;
    JDL::clear();
    for (debug = lines1.begin(); debug != lines1.end(); ++debug)
    {
        cin >> garbage;
        (*debug)->draw();
        JDL::flush();
    }

    for (debug = lines2.begin(); debug != lines2.end(); ++debug)
    {
        cin >> garbage;
        (*debug)->draw();
        JDL::flush();
    }
*/
    
    cout << "Finished the split" << endl;
}


void Shape::grabShapeLines(int startIndex, int endIndex, vector<Line*> *result)
{
    int index;
    if (result->size() > 0)
    {
        index = result->back()->index + 1;
    }

    vector<Line*>::iterator i = this->lines.begin() + startIndex;
    do {        
        if (i == this->lines.end())
        {
            i = this->lines.begin();
        }

        (*i)->index = index;
        index++;
        result->push_back(*i);

        ++i;
    } while ( i != (this->lines.begin() + endIndex + 1));
}

void appendLines(std::vector<Line*> *lines1, std::vector<Line*> &lines2)
{    
    Line *lastLine1 = lines1->back();
    int index = lastLine1->index + 1;

    Point lastPoint2 = lines2.back()->point2;
    Point firstPoint2 = lines2[0]->point1;
    
    //if the first point of the first line in lines2 
    //is on the last line of lines1
    if (lastLine1->on(firstPoint2))
    {
        //standard combine
        vector<Line*>::iterator i;
        for (i = lines2.begin(); i != lines2.end(); ++i)
        {
            (*i)->index = index;
            index++;
            lines1->push_back(*i);
        }
    }
    //if the last point of the last line in lines2
    //is on the last line of lines1
    else if (lastLine1->on(lastPoint2))
    {
        //reverse combine
        vector<Line*>::reverse_iterator i;
        for (i = lines2.rbegin(); i != lines2.rend(); ++i)
        {
            (*i)->index = index;
            index++;

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
            int tempIndex = (*debug)->index;
            (*debug)->index = 1;
            (*debug)->draw();
            (*debug)->index = tempIndex;
            JDL::flush();
        }

        JDL::circle(firstPoint2.x, firstPoint2.y, 5);
        JDL::circle(lastPoint2.x, lastPoint2.y, 5);


        for (debug = lines2.begin(); debug != lines2.end(); ++debug)
        {
            cin >> garbage;
            int tempIndex = (*debug)->index;
            (*debug)->index = 2;
            (*debug)->draw();
            (*debug)->index = tempIndex;
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

//return true if the shape is closed, return false otherwise.
//TODO: this is a quick sanity check. Figure 8s won't be checked.
bool Shape::sanityCheck()
{
    vector<Line*>:: iterator l;
    if (lines.size() == 0) return true; //empty shapes are fine
    if (lines.size() == 1)
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
            return false;
        }
        previous = *l;
    }
    
    //check that the last line connects with the first
    if (lines.back()->point2 != lines[0]->point1)
    {
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
        JDL::flush();
        JDL::sleep(1);
    }

    debugDraw();
}

void Shape::grabShapeLines(vector<Line*>::iterator startIt, 
                           vector<Line*>::iterator endIt, 
                           vector<Line*> *result)
{
    result->size();
    cout << *startIt;
    vector<Line*>::iterator l = startIt;
    result->push_back(*l);
    while (l != endIt)
    {
        l++;
        if (l == lines.end())
        {
            l = lines.begin();
        }
        result->push_back(*l);
        //cout << l - lines.begin() << endl;
    } 
}
