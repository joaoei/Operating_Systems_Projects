#ifndef POINT_H
#define POINT_H

struct Point {
    Point () {}
    Point (int _x, int _y) : x(_x), y(_y)
    {}

    int x;
    int y;

    bool inline operator==(const Point &rhs) {
        return (this->x == rhs.x and this->y == rhs.y);
    }
};


#endif // POINT_H
