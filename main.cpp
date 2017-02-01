#include "Shape.hpp"
#include "gfx_j.h"

#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char **argv)
{
    vector<Shape> toDraw;
    vector<Point> points;
    points.push_back(Point(100, 100));
    points.push_back(Point(100, 200));
    points.push_back(Point(200, 100));
    toDraw.push_back(Shape(points, &toDraw));

    gfx_open(800, 800, "Research");
    vector<Shape>::iterator i;
    char input = 'j'; //anything besides q
    while (input != 'q')
    {
        gfx_clear();
        for (i = toDraw.begin(); i != toDraw.end(); ++i)
        {
            if (input == 1)
            {
                i->fractureAt(Point(gfx_xpos(), gfx_ypos()));
            }
            i->draw();
        }
        gfx_flush();
        input = gfx_wait();
    }
}
