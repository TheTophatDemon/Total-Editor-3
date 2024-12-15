/**
 * Copyright (c) 2022-present Alexander Lunsford
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

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
#include "assets.hpp"

#define ENT_SPACING_DEFAULT 2.0f

// This is short for "entity", because "entity" is difficult to type.
struct Ent
{
    enum class DisplayMode {
        SPHERE,
        MODEL,
        SPRITE,
    };

    bool active;

    DisplayMode display;
    Color color;
    float radius;
    Vector3 lastRenderedPosition;
    int yaw, pitch; // Degrees angle orientation

    std::shared_ptr<Assets::ModelHandle> model;
    std::shared_ptr<Assets::TexHandle> texture;

    // Entity properties are key/value pairs. No data types are enforced, values are parsed as strings.
    std::map<std::string, std::string> properties; 

    Ent();
    Ent(float radius);

    void Draw(const bool drawAxes, const Vector3 position);
};

void to_json(nlohmann::json& j, const Ent &ent);
void from_json(const nlohmann::json& j, Ent &ent);

//This represents a grid of entities. 
//Instead of the grid storing entities directly, it stores iterators into the `_ents` array to save on memory.
class EntGrid : public Grid<Ent>
{
public:
    //Creates an empty entgrid of zero size
    EntGrid();

    //Creates ent grid of given dimensions, default spacing.
    EntGrid(size_t width, size_t height, size_t length);

    //Will set the given ent to occupy the grid space, replacing any existing entity in that space.
    inline void AddEnt(int i, int j, int k, Ent ent)
    {
        SetCel(i, j, k, ent);
    }

    inline void RemoveEnt(int i, int j, int k)
    {
        SetCel(i, j, k, Ent());
    }
    
    inline bool HasEnt(int i, int j, int k) const
    {
        return GetCel(i, j, k).active;
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
        assert(i + w <= int(_width) && j + h <= int(_height) && k + l <= int(_length));

        EntGrid newGrid(w, h, l);

        SubsectionCopy(i, j, k, w, h, l, newGrid);

        return newGrid;
    }

    //Returns a contiguous array of all active entities.
    inline std::vector<Ent> GetEntList() const
    {
        std::vector<Ent> out;
        for (const Ent &ent : _grid)
        {
            if (ent.active) out.push_back(ent);
        }
        return out;
    }

    void Draw(Camera &camera, int fromY, int toY);
    void DrawLabels(Camera &camera, int fromY, int toY);
private:
    std::vector<std::pair<Vector3, std::string>> _labelsToDraw;
};

#endif
