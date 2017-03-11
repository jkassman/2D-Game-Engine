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
    bool operator==(const Point &other) const;
    bool operator!=(const Point &other);
    double operator[](char coord);
    bool operator<(const Point &other) const;

    bool near(const Point &other, double radius);

    void drawCircle(int radius) const;
};

#endif
