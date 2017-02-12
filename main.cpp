#include "Shape.hpp"
#include "JDL.hpp"

#include <iostream>

#ifdef JDL_USE_SDL
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

int main(int argc, char **argv)
{
    vector<Shape*> toDraw;
    vector<Point> points;
    points.push_back(Point(100, 100));
    points.push_back(Point(100, 200));
    points.push_back(Point(200, 100));
    toDraw.push_back(new Shape(points, &toDraw));

    JDL::init(800, 800, "Research");
    vector<Shape*>::iterator i;
    vector<Point> newPoints;
    char input;
    int x, y;
    int mode = 0;
    Shape *building;
    bool quit = false;
    //bool first = true;
    int toDrawSize;
    int j;

    srand(time(NULL));

    while (!quit)
    {
    //get input
        /*
        if (first)
        {
            input = 'j';
            first = false;
        }
        else
        {
            input = JDL::wait(&x, &y);
        }
        */
        if (JDL::event_waiting())
        {
            input = JDL::wait(&x, &y);
        }
        else
        {
            input = 0;
        }

    //interpret input
        switch (input)
        {
        case 'q':
            quit = true;
            break;
        case 'b':
            mode = 1;
            building = new Shape(newPoints, &toDraw);
            toDraw.push_back(building);
            break;
        case 'f':
            mode = 0;
            break;
        case 1:
            //handle modes
            switch (mode)
            {
            case 1: //building mode
                building->addPoint(Point(x, y));
                break;
            case 0:
                toDrawSize = toDraw.size();
                for (j = 0; j < toDrawSize; j++)
                {
                    toDraw[j]->fractureAt(Point(x,y));
                }
            }
        }

        //cout << "About to draw stuff, sizeof toDraw: " <<toDraw.size() << endl;

    //draw and move shapes
        JDL::clear();
        for (i = toDraw.begin(); i != toDraw.end(); ++i)
        {
            (*i)->move();
            (*i)->draw();
        }
        JDL::flush();
        JDL::sleep(.03);
    }
/*
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
            i->draw();
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
*/
}
