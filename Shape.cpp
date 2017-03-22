#include "Shape.hpp"
#include "JDL.hpp"

#define _USE_MATH_DEFINES

#include <iostream>
#include <algorithm>
#include <math.h>
#include <fstream>
//#include <sstream>

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

//returns how many cracks were affected
int Shape::fractureAt(Point clickPoint)
{
    save("saves/Before_Click.txt");
    vector<Crack*> impactedCracks;
    vector<Line*>::iterator i;
    double force = 50;
    int numCracks = 0;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        Point result;
        if ((*i)->on(clickPoint, 10, &result))
        {
            //JDL::circle(result.x, result.y, 8);            
            numCracks += (*i)->getImpactedCracks(clickPoint, this,
                                                 &impactedCracks);
        }
    }
    if (numCracks == 0)
    {
        Crack *newCrack = addCrack(clickPoint);
        if (newCrack)
        {
            cout << "Creating New Crack" << endl;
            impactedCracks.push_back(newCrack);
        }
    }
    
    //now update the cracks that we have gathered.
    vector<Crack*>::iterator c;
    vector<Line*> splitLines;
    //cout << impactedCracks.size() << endl;
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
    //lines.back()->index = lines.size()-1;
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
void Shape::split(vector<Line*> &splitLines)
{
    //new new plan:
    //split the shape by lines, create two new shapes from them.
    //split startLine, split endLine, then throw the lines into shapes.

    //endLine splits intersectPoint->point2.
    vector<Line*> lines1, lines2;
    vector<Line*>::iterator l;
    vector<Crack*>::iterator c;
    
    Point crackStartPoint, crackEndPoint;
    crackStartPoint = splitLines[0]->point1;
    crackEndPoint = splitLines.back()->point2;
    if (crackStartPoint == crackEndPoint)
    {
        cerr << "PROBLEMS!!!!" << endl;
    }
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
    this->lines.clear();
    this->lines.assign(lines2.begin(), lines2.end());
    removeCracksOutside();
    
    if (!sanityCheck())
    {
        debugDraw();
    }


    /* Create a new shape (using lines1) */
    Shape *newShape = new Shape(lines1, this->toDraw);
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

    //Set all the cracks in the new shape to have their parent be newShape
    newShape->updateCrackParents();
    updateCrackParents();
                     

    //TODO: Delete splitlines here! We made two copies
    
    cout << "Finished the split" << endl;
    save("saves/After_Split.txt");
}

void Shape::updateCrackParents()
{
    vector<Line*>::iterator l;
    for (l = this->lines.begin(); l != this->lines.end(); ++l)
    {
        (*l)->setCrackParents(this);
    }
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

//return true if the shape is closed, return false otherwise.
//TODO: this is a quick sanity check. Figure 8s won't be checked.
bool Shape::sanityCheck()
{
    vector<Line*>::iterator l;
    vector<Crack*>::iterator c;
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        (*l)->sanityCheck(this);
    }

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

//returns the number of intersect points found
//ignores crackToIgnore
int Shape::lineIntersectsCrack(const Line &toCheck, 
                               vector<Crack*> *intersectCracks, 
                               vector<Point>  *intersectPoints,
                               Crack *crackToIgnore) const
{
    vector<Line*>::const_iterator l;
    vector<Crack*>::const_iterator c;
    int numIntersects = 0;
    Point *intersect = new Point();
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        for (c = (*l)->cracks.begin(); c != (*l)->cracks.end(); ++c)
        {
            if ((*c) == crackToIgnore)
            {
                continue;
            }
            if ((*c)->lineIntersects(toCheck, intersect))
            {
                intersectCracks->push_back(*c);
                intersectPoints->push_back(*intersect);

                numIntersects++;
                intersect = new Point();
            }
        }
    }
    delete intersect;
    return numIntersects;
}

void Shape::removeCracksOutside()
{
    vector<Line*>::iterator l;
    vector<Crack*>::iterator c;
    
    for (l = lines.begin(); l != lines.end(); ++l)
    {
        c = (*l)->cracks.begin();
        while (c != (*l)->cracks.end())
        {
            if (!inside((*c)->getFirstLine()->point2))
            {
                c = (*l)->cracks.erase(c);
            }
            else
            {
                ++c;
            }
        }
    }
}

bool Shape::inside(Point toTest)
{
    Line ray(toTest, 42, 0);
    int rayResult = rayTrace(ray);
    return (rayResult % 2); //if odd, inside, if even, outside.
}


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

void Shape::save(string fileName)
{
    ofstream outFile(fileName);
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
    ifstream inFile(fileName);
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

int Shape::saveNum = 0;

//destroys original list
//expects every item in the list to be surrounded by {}
void parseJsonList(string *jsonString, vector<string> *toFill)
{
    if (jsonString->front() != '[')
    {
        cerr << "Invalid JSON passed to parseJsonList" << endl;
        return;
    }
    jsonString->erase(0, 1); //erase the [

    while (jsonString->front() != ']')
    {
        if (jsonString->front() == ',')
        {
            jsonString->erase(0, 1);
        }
        toFill->push_back(grabJsonObject(jsonString));
        //cout << toFill->back() << endl;
    }
    jsonString->erase(0, 1); //erase the ]
}

//object must be surrounded by {} or []
string grabJsonObject(string *jsonString)
{
    if ((jsonString->front() != '{') && (jsonString->front() != '['))
    {
        cerr << "Invalid JSON passed to grabJsonObject!" << endl;
        return *jsonString;
    }
    int levelsDown = 0;
    string toReturn;
    do 
    {
        toReturn.push_back(jsonString->front());
        if ((jsonString->front() == '}') || (jsonString->front() == ']'))
        {
            levelsDown--;
        }
        else if ((jsonString->front() == '{') || (jsonString->front() == '['))
        {
            levelsDown++;
        }
        jsonString->erase(0, 1);
    } while (levelsDown > 0);
    return toReturn;
}

//does NOT modify jsonString
//returns "" if it cannot find the value in the dict
string grabJsonValue(string jsonString, string value)
{
    //scan to find a match
    int i, valueIndex;
    valueIndex = 0;
    for (i = 0; i < jsonString.length(); i++)
    {
        if (jsonString[i] == value[valueIndex])
        {
            valueIndex++;
        }
        else
        {
            valueIndex = 0;
        }
        if (valueIndex == value.length())
        {
            //found a match!
            break;
        }
    }

    if (valueIndex != value.length())
    {
        //could not find that value
        return "";
    }

    if ((jsonString[i + 1] != '\"') || (jsonString[i + 2] != ':'))
    {
        cerr << "Invalid JSON found in grabJsonValue" << endl;
        return "";
    }

    i += 3;

    int levelsDown = 0;
    string toReturn = "";

    while (true)
    {
        if (levelsDown == 0)
        {
            if ((jsonString[i] == '}') || (jsonString[i] == ','))
            {
                break;
            }
        }
        if ((jsonString[i] == '{') || (jsonString[i] == '['))
        {
            levelsDown++;
        }
        else if ((jsonString[i] == '}') || (jsonString[i] == ']'))
        {
            levelsDown--;
        }

        toReturn += jsonString[i];
        i++;
    }
    return toReturn;
}