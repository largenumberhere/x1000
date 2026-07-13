#include "ray_wrap/rectangle_shrink.h"

Rectangle rectangleShrink(Rectangle rectangle, float pixels) {
    rectangle.x += pixels ;
    rectangle.y += pixels ;
    rectangle.width -= pixels * 2;
    rectangle.height -= pixels * 2;

    return rectangle;
}