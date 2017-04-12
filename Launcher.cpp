#include <iostream>

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

    center = Point(width/2, height/2);

    move(600, 0);
    move(100, 90);
    this->toDraw = toDraw;

    toDraw->push_back((Shape*) this);
}

Point Launcher::getPosition()
{
    return center;
}

void Launcher::fire()
{
    vector<Line*> projectileLines;
    createProjectileLines(Point(center.x - width/2, center.y), 10, 10,
                          &projectileLines);
    Shape *projectile = new Shape(projectileLines, toDraw);
    projectile->setBounds(0, 800, 0, 800);
    projectile->setBoundType(SHAPE_BOUND_BOUNCE);
    projectile->setVelocity(10, 180);
    projectile->projectile = true;
    
    cout << "ATTACK" << endl;
    toDraw->push_back(projectile);
}

void Launcher::move(double distance, double direction)
{
    center.move(distance, direction);
    ((Shape*) this)->move(distance, direction);
}

void Launcher::createProjectileLines(Point center, int numSides, int sideSize,
                                     vector<Line*> *toFill)
{
    int i;
    Point startPoint(center.x - width/2, center.y - sideSize/2);
    toFill->push_back(new Line(startPoint, sideSize, 0));
    for (i = 1; i < numSides; i++)
    {
        toFill->push_back(new Line(toFill->back()->point2, sideSize,
                                   360.0/numSides*i));
    }
}
