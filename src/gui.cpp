#include "gui.hpp"

std::vector<Rectangle> ArrangeHorzCentered(Rectangle bounds, std::initializer_list<Rectangle> rects)
{
    const float boundsHH = bounds.height / 2.0f;

    const float OFFSET = bounds.width / (rects.size() + 1);
    std::vector<Rectangle> out = rects;
    float xCenter = OFFSET;
    for (Rectangle &r : out)
    {
        r.x = bounds.x + xCenter - (r.width / 2.0f);
        r.y = bounds.y + boundsHH - (r.height / 2.0f);
        xCenter += OFFSET;
    }

    return out;
}

std::vector<Rectangle> ArrangeVertical(Rectangle area, std::initializer_list<Rectangle> rects)
{
    const float REGION_HEIGHT = area.height / rects.size();

    std::vector<Rectangle> out = rects;
    int i = 0;;
    for (Rectangle& r : out)
    {
        r.x += area.x;
        r.y += area.y + (i * REGION_HEIGHT) + (REGION_HEIGHT / 2.0f) - (r.height / 2.0f);
        ++i;
    }
    return out;
}

std::vector<Rectangle> ArrangeVertical(Rectangle area, Rectangle templ, int count)
{
    const float REGION_HEIGHT = area.height / count;

    std::vector<Rectangle> out(count);
    int i = 0;
    for (Rectangle& r : out)
    {
        r = templ;
        r.x += area.x;
        r.y += area.y + (i * REGION_HEIGHT) + (REGION_HEIGHT / 2.0f) - (r.height / 2.0f);
        ++i;
    }
    return out;
}