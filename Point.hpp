#ifndef POINT_HPP
#define POINT_HPP

//simple 2D point.
class Point
{
private:

public:
    int x, y;

    Point();
    Point(int x, int y);
    Point(const Point &other);
	void print();
    Point operator=(const Point &other);
    bool operator==(const Point &other);
    int operator[](char coord);
};

#endif
