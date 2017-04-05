#ifndef ND_RESEARCH_LAUNCHER_HPP
#define ND_RESEARCH_LAUNCHER_HPP

#include "Shape.hpp"

class Launcher : public Shape
{
private:
    std::vector<Shape*> *toDraw;
    Point center;
    int width;
    int height;
public:
    Launcher(std::vector<Shape*> *toDraw);
    
    Point getPosition();
    void fire();
    void move(double distance, double direction);
    void createProjectileLines(Point center, int numSides, 
                               int sideSize, std::vector<Line*> *toFill);
};

#endif
