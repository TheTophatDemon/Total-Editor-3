#ifndef ENT_H
#define ENT_H

#include "raylib.h"
#include "raymath.h"
#include "json.hpp"

#include <vector>
#include <string>
#include <map>

#include "grid.hpp"
#include "math_stuff.hpp"

#define ENT_SPACING_DEFAULT 2.0f

//This is short for "entity", because "entity" is difficult to type.
struct Ent
{
    Color color;
    float radius;
    Vector3 position; //World space coordinates
    int yaw, pitch; //Degrees angle orientation

    //Entity properties are key/value pairs. No data types are enforced, values are parsed as strings.
    std::map<std::string, std::string> properties; 

    inline Matrix GetMatrix() const
    {
        return MatrixMultiply(
            MatrixMultiply(
                MatrixRotateX(ToRadians((float) pitch)), 
                MatrixRotateY(ToRadians((float) yaw))
            ),
            MatrixTranslate(position.x, position.y, position.z));
    }

    //Entity is considered empty if the radius is zero;
    inline operator bool() const
    {
        return radius != 0.0f;
    }

    void Draw() const;
};

void to_json(nlohmann::json& j, const Ent &ent);
void from_json(const nlohmann::json& j, Ent &ent);

//This represents a grid of entities. 
//Instead of the grid storing entities directly, it stores iterators into the `_ents` array to save on memory.
class EntGrid : public Grid<Ent>
{
public:
    //Creates an empty entgrid of zero size
    inline EntGrid()
        : EntGrid(0, 0, 0)
    {
    }

    //Creates ent grid of given dimensions, default spacing.
    inline EntGrid(size_t width, size_t height, size_t length)
        : Grid<Ent>(width, height, length, ENT_SPACING_DEFAULT, (Ent) { 0 }) //Nullptr means no entity in the cel
    {
    }

    //Will set the given ent to occupy the grid space, replacing any existing entity in that space.
    inline void AddEnt(int i, int j, int k, const Ent &ent)
    {
        SetCel(i, j, k, ent);
    }

    inline void RemoveEnt(int i, int j, int k)
    {
        SetCel(i, j, k, (Ent) { 0 });
    }
    
    inline bool HasEnt(int i, int j, int k) const
    {
        return GetCel(i, j, k);
    }

    inline Ent GetEnt(int i, int j, int k) const
    {
        assert(HasEnt(i, j, k));
        return GetCel(i, j, k);
    }

    inline void CopyEnts(int i, int j, int k, const EntGrid &src)
    {
        return CopyCels(i, j, k, src);
    }

    //Returns a smaller grid with a copy of the ent data in the rectangle defined by coordinates (i, j, k) and size (w, h, l).
    inline EntGrid Subsection(int i, int j, int k, int w, int h, int l) const
    {
        assert(i >= 0 && j >= 0 && k >= 0);
        assert(i + w <= _width && j + h <= _height && k + l <= _length);

        EntGrid newGrid(w, h, l);

        SubsectionCopy(i, j, k, w, h, l, newGrid);

        return newGrid;
    }

    //Returns a continuous array of all active entities.
    inline std::vector<Ent> GetEntList() const
    {
        std::vector<Ent> out;
        for (const Ent &ent : _grid)
        {
            if (ent) out.push_back(ent);
        }
        return out;
    }

    void Draw(Camera &camera, int fromY, int toY);
    void DrawLabels(Camera &camera, int fromY, int toY);
};

#endif