#ifndef POINT_HPP
#define POINT_HPP

#include <string>

//simple 2D point.
class Point
{
private:

public:
    double x, y;

    Point();
    Point(double x, double y);
    Point(const Point &other);
    Point(std::string jsonString);
    void print() const;
    Point operator=(const Point &other);
    bool operator==(const Point &other) const;
    bool operator!=(const Point &other);
    double operator[](char coord);
    bool operator<(const Point &other) const;

    bool near(const Point &other, double radius);
    void translate(double distance, double radians);
    void rotateAbout(double theta, Point about);

    void drawCircle(int radius) const;

    std::string generateJSON() const;
};

#endif
