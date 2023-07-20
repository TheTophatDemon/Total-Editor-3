#ifndef GUI_H
#define GUI_H

#include <vector>

#include "raylib.h"

//Takes some rectangles and returns copies of them, repositioned to be horizontally laid out inside of the `area` rectangle.
std::vector<Rectangle> ArrangeHorzCentered(Rectangle bounds, std::initializer_list<Rectangle> rects);

//Takes some rectangles and returns copies of them, repositioned to be vertically laid out inside of the `area` rectangle.
std::vector<Rectangle> ArrangeVertical(Rectangle area, std::initializer_list<Rectangle> rects);

//Takes some rectangles, that are all the same size, and returns copies of them, repositioned to be vertically laid out inside of the `area` rectangle.
std::vector<Rectangle> ArrangeVertical(Rectangle area, Rectangle templ, int count);

#endif