#include "Launcher.hpp"
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

    const int WIDTH = 800;
    const int HEIGHT = 800;

    JDL::init(WIDTH, HEIGHT, "Research");
    vector<Shape*>::iterator i;
    vector<Point> newPoints;
    char input;
    int x, y;
    int mode = 0;


    Launcher launchy(&toDraw);
    double launchMoveSpeed = 5;
    double launchMoveDirection = 90;

    Shape *building = NULL;
    bool quit = false;
    //bool first = true;
    int toDrawSize;
    int j;
#ifndef JDL_USE_SDL
    srand(time(NULL));
#endif
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
        vector <Line*> newLines;
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
        case 'a':
            launchy.fire();
            break;
        case 'l':
            toDraw.clear(); //memory leaks yay
            loadLines("toLoad.txt", &newLines);
            toDraw.push_back(new Shape(newLines, &toDraw));
            toDraw.back()->updateCrackParents();
        case 1:
            //handle modes
            switch (mode)
            {
            case 1: //building mode
                building->addPoint(Point(x, y));
                break;
            case 0:
                //cout << "clicked!" << endl;
                toDrawSize = toDraw.size();
                for (j = 0; j < toDrawSize; j++)
                {
                    if (toDraw[j]->fractureAt(Point(x,y)))
                    {
                        break;
                    }
                }
                //cout << "time to draw!" << endl;
                break;
            }
        }

        //cout << "About to draw stuff, sizeof toDraw: " <<toDraw.size() << endl;

    //draw and move shapes
        JDL::clear();
        if (launchy.getPosition().y <= 100)
        {
            launchMoveDirection = 90;
        }
        if (launchy.getPosition().y >= (HEIGHT - 100))
        {
            launchMoveDirection = -90;
        }
        launchy.move(launchMoveSpeed, launchMoveDirection);


        for (i = toDraw.begin(); i != toDraw.end(); ++i)
        {
            (*i)->move();
            (*i)->collide();
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
