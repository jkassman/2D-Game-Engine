#ifndef POINT_HPP
#define POINT_HPP

//simple 2D point.
class Point
{
private:

public:
    double x, y;

    Point();
    Point(double x, double y);
    Point(const Point &other);
    void print() const;
    Point operator=(const Point &other);
    bool operator==(const Point &other);
    int operator[](char coord);
    bool operator<(const Point &other) const;
};

#endif
