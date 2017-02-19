#include "Shape.hpp"
#include "JDL.hpp"

#include <iostream>
#include <algorithm>

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

void Shape::fractureAt(Point clickPoint)
{
    vector<Crack*> impactedCracks;
    vector<Line*>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        Point result;
        if ((*i)->on(clickPoint, 10, &result))
        {
            //JDL::circle(result.x, result.y, 8);
            //
            double force = 21;
            
            (*i)->getImpactedCracks(result, this, force, &impactedCracks);

            //
            if ((*i)->increaseCracks(result, this, 21))
            {
                return;
            }
        }
    }
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
    Line *start1 = new Line();
    Line *start2 = new Line();
    int start1Index, start2Index;
    vector<Line*> lines1, lines2;

    start1Index = endLine->index;
    start2Index = startLine->index;
    if (startLine == endLine)
    {
        Point farPoint, closePoint; //far or close to startLine->point1
        Line firstSplit(startLine->point1, splitLines[0]->point1);
        Line secondSplit(startLine->point1, splitLines.back()->point2);
        if (firstSplit.length() > secondSplit.length())
        {
            farPoint = splitLines[0]->point1;
            closePoint = splitLines.back()->point2;
        }
        else
        {
            farPoint = splitLines.back()->point2;
            closePoint = splitLines[0]->point1;
        }
        
        startLine->split(closePoint, start2);
        cout << "context" << endl;
        //startline: point1->closePoint
        //start2: closePoint->point2.
        
        start2->split(farPoint, start1);
        cout << "context" << endl;
        //start2: closePoint->farPoint
        //start1: farPoint->point2
    }
    else
    {
        endLine->split(splitLines.back()->point2, start1);
        //endLine is now endLine.point1->intersectPoint
        
        startLine->split(splitLines[0]->point1, start2);
        //startLine is now startLine.point1->intersectPoint
    }
    start1->index = 0;
    start2->index = 0;
    int newIndex = 1;

    lines1.push_back(start1);
    lines2.push_back(start2);

    //now we want all lines in the shape between start1 and startLine.
    //let's assume indexes work.
    //cout << "start indexes" << start1Index << "," << start2Index << endl;

    vector<Line*>::iterator i = this->lines.begin() + start1Index;
    //Note: i++ works because shape is constructed point1->point2->point1 etc.


    //This should grab ~half the lines in the shape
    do 
    {
        ++i;
        if (i == this->lines.end())
        {
            i = this->lines.begin();
        }

        (*i)->index = newIndex;
        newIndex++;
        lines1.push_back(*i);
    } while (i != this->lines.begin() + start2Index);

    //re-assign all the lines in the crack
    vector<Line*>::iterator j;

    for (j = splitLines.begin(); j != splitLines.end(); ++j)
    {
        (*j)->index = newIndex;
        newIndex++;
        lines1.push_back(*j);
    }

    //finished with lines1!

    //now create lines2.
    newIndex = 1;
    i = this->lines.begin() + start2Index;
    while (i != this->lines.begin() + start1Index)
    {
        ++i;
        if (i == this->lines.end())
        {
            i = this->lines.begin();
        }

        (*i)->index = newIndex;
        newIndex++;
        lines2.push_back(*i);
    }

    //actually copy all the lines in the crack
    //we should reverse them as well.
    vector<Line*>::reverse_iterator r;
    for (r = splitLines.rbegin(); r != splitLines.rend(); ++r)
    {
        lines2.push_back(new Line(**r));
        lines2.back()->index = newIndex;
        newIndex++;

        //switch point1 and point2.
        lines2.back()->switchPoints();
    }

    this->lines.clear();
    this->lines.assign(lines2.begin(), lines2.end());

    //ok, now create a shape out of all that. Move it away, too.
    toDraw->push_back(new Shape(lines1, this->toDraw));
    int newDirection = 0;

    Line crackDirection(splitLines[0]->point1, splitLines.back()->point2);
    newDirection = crackDirection.getDirection() - 90;
    toDraw->back()->setSpeed(0.42 + this->speed, newDirection);
                     

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
