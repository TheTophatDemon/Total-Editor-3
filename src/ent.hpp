#ifndef ENT_H
#define ENT_H

#include "raylib.h"
#include "raymath.h"

#include <map>
#include <string>

#include "grid.hpp"
#include "math_stuff.hpp"

#define ENT_SPACING_DEFAULT 2.0f

//This is short for "entity", because "entity" is difficult to type.
struct Ent
{
    Model *model; //Display model
    Texture2D *sprite; //If there's not a model, it will make a billboard using this texture.
    //If both model and sprite are nullptr, then a group of axes is drawn instead to represent an "empty" entity.
    Vector3 position;
    int yaw, pitch; //Degrees angle orientation
    std::map<std::string, std::string> properties; //Entity properties are key/value pairs. No data types are enforced, values are parsed as strings.
    
    inline Matrix GetMatrix() const
    {
        return MatrixMultiply(
            MatrixMultiply(
                MatrixRotateX(ToRadians((float) pitch)), 
                MatrixRotateY(ToRadians((float) yaw))
            ),
            MatrixTranslate(position.x, position.y, position.z));
    }

    void Draw(Camera &camera) const;
};

//This represents a grid of entities. 
//Instead of the grid storing entities directly, it stores iterators into the `_ents` array to save on memory.
class EntGrid : public Grid<const Ent *>
{
public:
    //Creates an empty entgrid of zero size
    inline EntGrid()
        : EntGrid(0, 0, 0)
    {
    }

    //Creates ent grid of given dimensions, default spacing.
    inline EntGrid(size_t width, size_t height, size_t length)
        : Grid<const Ent *>(width, height, length, ENT_SPACING_DEFAULT, nullptr) //Nullptr means no entity in the cel
    {
    }

    //Will set the given ent to occupy the grid space, replacing any existing entity in that space.
    inline void AddEnt(int i, int j, int k, const Ent &ent)
    {
        RemoveEnt(i, j, k);
        _ents.push_back(ent);
        SetCel(i, j, k, &_ents.back());
    }

    inline void RemoveEnt(int i, int j, int k)
    {
        if (const Ent *oldEnt = GetCel(i, j, k); oldEnt != nullptr)
        {
            for (auto e = _ents.cbegin(); e != _ents.cend(); ++e) 
            {
                if (e.base() == oldEnt)
                {
                    _ents.erase(e);
                    break;
                }
            }
            SetCel(i, j, k, nullptr);
        }
    }
    
    inline bool HasEnt(int i, int j, int k) const
    {
        return GetCel(i, j, k) != nullptr;
    }

    inline Ent GetEnt(int i, int j, int k) const
    {
        assert(HasEnt(i, j, k));
        return *GetCel(i, j, k);
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

    void Draw(Camera &camera, int fromY, int toY);
protected:
    std::vector<Ent> _ents; //This is a continuous list of active entities, that is updated as the grid is modified.
};

#endif