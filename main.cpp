#include "Launcher.hpp"
#include "Shape.hpp"
#include "JDL.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

#ifdef JDL_USE_SDL
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

int main(int argc, char **argv)
{
    const int WIDTH = 800;
    const int HEIGHT = 800;

    JDL::init(WIDTH, HEIGHT, "Research");


    vector<Shape*> toDraw;
    vector<Point> points;
    points.push_back(Point(100, 100));
    points.push_back(Point(100, 200));
    points.push_back(Point(200, 100));
    toDraw.push_back(new Shape(points, &toDraw));
    toDraw.back()->setBoundType(SHAPE_BOUND_BOUNCE);
    toDraw.back()->setBounds(0, 800, 0, 800);
    toDraw.back()->setAngularVelocity(0);

    vector<Shape*>::iterator i;
    vector<Point> newPoints;
    char input;
    int x, y;
    int mode = 0;

    bool splitOccurred = false;

    Launcher launchy(&toDraw);
    launchy.setVelocity(4, 90);
    //double launchMoveSpeed = 5;
    //double launchMoveDirection = 90;

    Shape *building = NULL;
    bool quit = false;
    //bool first = true;
    int toDrawSize;
    int j;
#ifndef JDL_USE_SDL
    srand(time(NULL));
#endif
    int storyNum = 0;
    bool stop = false;
    string filename;
        
    while (!quit)
    {
        double moveDist = 50;
        vector<Shape*>::iterator s;

        if (stop)
        {
            char next = gfx_wait();
            stringstream streamy;
            switch(next)
            {
            case 'n':
                //load story
                storyNum++;
                streamy << "story/save" << storyNum << ".txt";
                toDraw.clear(); //TODO: memory leaks yay
                cout << "trying to load from " << streamy.str() << "." << endl;
                loadShapes(streamy.str(), &toDraw);
                streamy.clear();
                break;
            case 'p':
                //load previous story
                storyNum--;
                streamy << "story/save" << storyNum << ".txt";

                toDraw.clear(); //TODO: Memory leaks
                cout << "trying to load from " << streamy.str() << "." << endl;
                loadShapes(streamy.str(), &toDraw);
                streamy.clear();
                break;
            case '=':
                //zoom in
                for (s = toDraw.begin(); s != toDraw.end(); ++s)
                {
                    (*s)->scale(2);
                }
                break;
            case '-':
                //zoom out
                for (s = toDraw.begin(); s != toDraw.end(); ++s)
                {
                    (*s)->scale(0.5);
                }
                break;
            case '1':
                //move left
                for (s = toDraw.begin(); s != toDraw.end(); ++s)
                {
                    (*s)->translate(moveDist, 180);
                }
                break;

            case '2':
                //move right
                for (s = toDraw.begin(); s != toDraw.end(); ++s)
                {
                    (*s)->translate(moveDist, 0);
                }
                break;
            case '3':
                //move down
                for (s = toDraw.begin(); s != toDraw.end(); ++s)
                {
                    (*s)->translate(moveDist, 90);
                }
                break;
            case '4':
                //move up
                for (s = toDraw.begin(); s != toDraw.end(); ++s)
                {
                    (*s)->translate(moveDist, -90);
                }
                break;
            case 's':
                stop = false;
                break;
            default:
                stop = true;
                break;
            }
            JDL::clear();
            for (s = toDraw.begin(); s != toDraw.end(); ++s)
            {
                (*s)->draw();
            }
            JDL::flush();
            if (stop) continue;
        }
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
            building->setBoundType(SHAPE_BOUND_BOUNCE);
            building->setBounds(0, 800, 0, 800);
            toDraw.push_back(building);
            break;
        case 'f':
            mode = 0;
            break;
        case 'a':
            launchy.fire();
            break;
        case 's':
            //stop everything
            /*
            for (s = toDraw.begin(); s != toDraw.end(); ++s)
            {
                (*s)->setVelocity(0, 0);
            }
            */
            stop = true;
            break;
        case 'o':
            toDraw.clear(); //TODO: memory leaks yay
            loadShapes("toLoad.txt", &toDraw);
            break;
        case 'l':
            toDraw.clear(); //TODO: memory leaks yay
            toDraw.push_back(loadShape("toLoad.txt", &toDraw));
            break;
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
                    if (toDraw[j]->fractureAt(Point(x,y), 25))
                    {
                        break;
                    }
                }
                //TODO: This is going to be hacky
                splitOccurred = true;
#ifdef RESEARCH_SAVE_STORY_SHORT
                if (splitOccurred)
                {
                    toDraw[0]->saveStory();
                }
#endif

                while (splitOccurred)
                {
                    splitOccurred = false;
                    for (s = toDraw.begin(); s != toDraw.end(); ++s)
                    {
                        if ((*s)->tryOneSplit())
                        {
                            splitOccurred = true;
                            break;
                        }
                    }
                }
               
                //saveShapes("saves/All_After_Split.txt", &toDraw);
                //cout << "time to draw!" << endl;
                break;
            }
        }

        //cout << "About to draw stuff, sizeof toDraw: " <<toDraw.size() << endl;

    //draw and move shapes
        JDL::clear();
        /*
        if (launchy.getPosition().y <= 100)
        {
            launchMoveDirection = 90;
        }
        if (launchy.getPosition().y >= (HEIGHT - 100))
        {
            launchMoveDirection = -90;
        }
        launchy.move(launchMoveSpeed, launchMoveDirection);
        */
        for (i = toDraw.begin(); i != toDraw.end(); ++i)
        {
            (*i)->move();
            (*i)->checkBounds();
            if ((*i)->collide()) splitOccurred = true;
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
