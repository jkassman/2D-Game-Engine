#include "Shape.hpp"
#include "JDL.hpp"

#include <iostream>
#include <algorithm>

using namespace std;

Shape::Shape(vector<Point> givenPoints, vector<Shape> *allShapes)
{
    this->allShapes = allShapes;
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

Shape::Shape(vector<Line*> &givenLines, vector<Shape> *allShapes)
{
    this->allShapes = allShapes;
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
    vector<Line*>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        Point result;
        if ((*i)->on(clickPoint, 10, &result))
        {
            //JDL::circle(result.x, result.y, 8);
            (*i)->increaseCracks(result, this, 1000);
            return;
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
void Shape::split(int indexA1, char posOrNegA, int indexA2, const std::vector<Line*> &CrackA,
                  int indexB1, char posOrNegB, int indexB2, const std::vector<Line*> &CrackB)
{
    /*
    //new new plan:
    //split the shape by lines, create two new shapes from them.
    //split startLine, split endLine, then throw the lines into shapes.

    //endLine splits intersectPoint->point2.
    Line start1, start2;
    int start1Index, start2Index;
    vector<Line> lines1, lines2;

    start1Index = endLine->index;
    endLine->split(lines.back().point2, &start1);
    //endLine is now endLine.point1->intersectPoint

    start2Index = startLine->index;
    startLine->split(lines[0].point1, &start2);
    //startLine is now startLine.point1->intersectPoint

    start1.index = 0;
    start2.index = 0;
    int newIndex = 1;

    lines1.push_back(start1);
    lines2.push_back(start2);

    //now we want all lines in the shape between start1 and startLine.
    //let's assume indexes work.
    //cout << "start indexes" << start1Index << "," << start2Index << endl;

    vector<Line>::iterator i = parentShape->lines.begin() + start1Index;
    //Note: i++ works because shape is constructed point1->point2->point1 etc.
    cout << "Pre first loop" << endl;
    lines[0].point2.print();
    cout << start2Index << endl;
    //This should grab ~half the lines in the shape
    while (i != (parentShape->lines.begin() + start2Index))
    {
        ++i;
        if (i == parentShape->lines.end())
        {
            i = parentShape->lines.begin();
        }
        i->index = newIndex;
        newIndex++;

        lines1.push_back(*i);
        //i = parentShape->lines.erase(i);
    }
    cout << "Pre second loop, line size: " << lines.size() << endl;
    //This copies all the lines in the crack
    vector<Line>::iterator j;

    for (j = lines.begin(); j != lines.end(); ++j)
    {
        j->index = newIndex;
        newIndex++;
        lines1.push_back(*j);
    }
    cout << "Shape creation" << endl;
    //ok, now create a shape out of all that. Move it away, too.
    //parentShape->allShapes->push_back(Shape(lines1, parentShape->allShapes));
    //parentShape->allShapes->back().move(150, 0);
    Shape newShape(lines1, parentShape->allShapes);
    newShape.move(150, 0);
    newShape.draw();
    cout << "Finished the split" << endl;
    */
}