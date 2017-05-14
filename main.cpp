#include "Launcher.hpp"
#include "Shape.hpp"
#include "JDL.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>

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
    const double TIMESTEP = 0.03;

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
    launchy.setVelocity(40, M_PI);

    Shape *building = NULL;
    bool quit = false;
    int toDrawSize;
    int j;
#ifndef JDL_USE_SDL
    srand(time(NULL));
#endif
    int storyNum = 0;
    bool stop = false;
    string filename;
    clock_t cycleEnd = clock();
    while (!quit)
    {
        clock_t cycleStart = clock();
        
        double moveDist = 50;
        vector<Shape*>::iterator s;

        if (stop)
        {
            int dummy;
            char next = JDL::wait(&dummy, &dummy);
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
        case 'd':
            //delete all shapes with less mass than .3
            for (i = toDraw.begin(); i != toDraw.end(); ++i)
            {
                if ((*i)->getMass() < .03)
                {
                    (*i)->toDelete = true;
                }
            }
            break;
        case 'f':
            mode = 0;
            break;
        case 'a':
            launchy.fire();
            break;
        case 's':
            //stop everything
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
                break;
            }
        }
    //draw and move shapes
        JDL::clear();
        for (i = toDraw.begin(); i != toDraw.end(); ++i)
        {
            clock_t moveClock = clock();
            double toMove = JDL::timeBetweenClocks(moveClock, cycleEnd);
            if (toMove > TIMESTEP) toMove = TIMESTEP;
            (*i)->move(toMove);
            (*i)->checkBounds();
            if ((*i)->collide()) splitOccurred = true;
            (*i)->draw();
        }

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
        splitOccurred = false;

        JDL::flush();
        cycleEnd = clock();
        double toSleep = TIMESTEP - JDL::timeBetweenClocks(cycleEnd, cycleStart);
        if (toSleep < 0)
        {
            cout << "Running " << -toSleep << " seconds behind! Skipping sleep" << endl;
        }
        else
        {
            JDL::sleep(toSleep);
        }

        //delete bad shapes from toDraw:
        for (s = toDraw.begin(); s != toDraw.end();)
        {
            if ((*s)->toDelete)
            {
                cout << "Erasing a shape!" << endl;
                s = toDraw.erase(s);
            }
            else
            {
                s++;
            }
        }
    }
}
