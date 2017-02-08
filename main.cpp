#include "Shape.hpp"
#include "JDL.hpp"

#include <iostream>

#ifdef SDL_USE_JDL
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

int main(int argc, char **argv)
{
    vector<Shape> toDraw;
    vector<Point> points;
    points.push_back(Point(100, 100));
    points.push_back(Point(100, 200));
    points.push_back(Point(200, 100));
    toDraw.push_back(Shape(points, &toDraw));

    JDL::init(800, 800, "Research");
    vector<Shape>::iterator i;
    char input = 'j'; //anything besides q or 1
    int x, y;
    int mode = 0;
    int building = -1;

    while (input != 'q')
    {
        //draw everything in toDraw
        JDL::clear();
        for (i = toDraw.begin(); i != toDraw.end(); ++i)
        {
            switch (mode)
            {
            case 0:
                if (input == 1)
                {
                    i->fractureAt(Point(x, y));
                }
                break;
            }
            cout << "Drawing" << endl;
            i->draw();
            cout << "Finished Drawing" << endl;
        }
        JDL::flush();

        input = JDL::wait(&x, &y);
        if (input == 'b')
        {
            mode = 1;
            vector<Point> newPoints;
            //newPoints.push_back(Point(x, y));
            building = toDraw.size();
            toDraw.push_back(Shape(newPoints, &toDraw));
        }
        if (input == 'f') //finished building
        {
            mode = 0;
        }

        switch (mode)
        {
        case 1: //building mode
            if (input == 1)
            {
                toDraw[building].addPoint(Point(x, y));
            }
        }
    }
}
