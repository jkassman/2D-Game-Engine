#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>


#include "Launcher.hpp"

using namespace std;

Launcher::Launcher(vector<Shape*> *toDraw) : Shape(toDraw)
{
    width = 142;
    height = 50;

    addPoint(Point(0,0));
    addPoint(Point(width, 0));
    addPoint(Point(width, height));
    addPoint(Point(0, height));

    setBounds(0, 800, 0, 800);
    setBoundType(SHAPE_BOUND_BOUNCE);

    translate(600, 0);
    translate(100, M_PI/2);
    this->toDraw = toDraw;
    orientation = M_PI;
    toDraw->push_back((Shape*) this);
}

Point Launcher::getPosition()
{
    return center;
}

void Launcher::fire()
{
    vector<Line*> projectileLines;
    
    Line distLine(center, width/2 + 30, orientation);
    createProjectileLines(distLine.point2, 5, 20,
                          &projectileLines);
    Shape *projectile = new Shape(projectileLines, toDraw);
    projectile->setBounds(0, 800, 0, 800);
    projectile->setBoundType(SHAPE_BOUND_BOUNCE);
    projectile->setVelocity(142, orientation);
    projectile->projectile = true;
    
    cout << "ATTACK" << endl;
    toDraw->push_back(projectile);
}

void Launcher::createProjectileLines(Point startPoint, int numSides, 
                                     int sideSize, vector<Line*> *toFill)
{
    int i;
    //Point startPoint(center.x - width/2, center.y - sideSize/2);
    toFill->push_back(new Line(startPoint, sideSize, 0));
    for (i = 1; i < numSides; i++)
    {
        toFill->push_back(new Line(toFill->back()->point2, sideSize,
                                   M_PI*2/numSides*i));
    }
}
