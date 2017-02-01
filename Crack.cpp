#include "Crack.hpp"

#define _USE_MATH_DEFINES

#include <math.h>
#include <iostream>

using namespace std;

Crack::Crack(Shape *parent, Point startPoint, Line startLine)
{
    parentShape = parent;
    line = startLine;
    points.push_back(startPoint);
}

void Crack::increase(int force)
{
    //ideas: create a line with a random slope starting from the given point.
    //check all lines in the shape to see if any intersect
    //(disregard the line the fracture point is on)
    //if multiple lines intersect, pick the closest line...smallest distance line?
    //if no lines intersect, choose a new random line

	int radius = force * 100;
	int numIntersects = 0;
	vector<Point> intersectPoints;
	while (numIntersects == 0)
	{
		//create a random slope of the line
		int degree = rand() % 360;
		//double slope = tan(degree*M_PI / 180);
		Point nextPoint(points[0].x + cos(degree*M_PI / 180) * radius, points[0].y + sin(degree*M_PI / 180) * radius);
		Line fractureLine(points[0], nextPoint);

		//check if the fractureLine intersects with any lines in the shape
		vector<Line>::iterator i;
		Point intersectPoint, dummyPoint;
		for (i = parentShape->lines.begin(); i != parentShape->lines.end(); ++i)
		{
			if (i->intersects(fractureLine, &intersectPoint))
			{
				if (!line.pointWithin(intersectPoint, 1, &dummyPoint))
				{
					numIntersects++;
					intersectPoints.push_back(intersectPoint);
				}
			}
		}
	}
	Line(points[0], intersectPoints[0]).draw();

	
}
