/**
 * Copyright (c) 2022 Alexander Lunsford
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

#ifndef GRID_H
#define GRID_H

#include <stdlib.h>
#include <vector>
#include <assert.h>

#include "math_stuff.hpp"

//Represents a 3 dimensional array of tiles and provides functions for converting coordinates.
template<class Cel>
class Grid
{
public:
    //Constructs a grid filled with the given cel.
    inline Grid(size_t width, size_t height, size_t length, float spacing, const Cel &fill)
    {
        _width = width; _height = height; _length = length; _spacing = spacing;
        _grid.resize(width * height * length);

        for (size_t i = 0; i < _grid.size(); ++i) { _grid[i] = fill; }
    }

    //Constructs a grid full of default-constructed cels.
    inline Grid(size_t width, size_t height, size_t length, float spacing)
        : Grid(width, height, length, spacing, Cel())
    {
    }

    //Constructs a blank grid of zero size.
    inline Grid()
        : Grid(0, 0, 0, 0.0f)
    {
    }

    inline Vector3 WorldToGridPos(Vector3 worldPos) const 
    {
        return Vector3{ floorf(worldPos.x / _spacing), floorf(worldPos.y / _spacing) , floorf(worldPos.z / _spacing)};
    }

    //Converts (whole number) grid cel coordinates to world coordinates.
    //If `center` is true, then the world coordinate will be in the center of the cel instead of the corner.
    inline Vector3 GridToWorldPos(Vector3 gridPos, bool center) const 
    {
        if (center) 
        {
            return Vector3 {
                (gridPos.x * _spacing) + (_spacing / 2.0f),
                (gridPos.y * _spacing) + (_spacing / 2.0f),
                (gridPos.z * _spacing) + (_spacing / 2.0f),
            };
        } 
        else 
        {
            return Vector3{ gridPos.x * _spacing, gridPos.y * _spacing, gridPos.z * _spacing };
        }
    }

    inline Vector3 SnapToCelCenter(Vector3 worldPos) const 
    {
        worldPos.x = (floorf(worldPos.x / _spacing) * _spacing) + (_spacing / 2.0f);
        worldPos.y = (floorf(worldPos.y / _spacing) * _spacing) + (_spacing / 2.0f);
        worldPos.z = (floorf(worldPos.z / _spacing) * _spacing) + (_spacing / 2.0f);
        return worldPos;
    }

    inline size_t FlatIndex(int i, int j, int k) const 
    {
        return i + (k * _width) + (j * _width * _length);
    }

    inline Vector3 UnflattenIndex(size_t idx) const 
    {
        return Vector3{
            (float)(idx % _width),
            (float)(idx / (_width * _length)),
            (float)((idx / _width) % _length)
        };
    }

    inline size_t GetWidth() const { return _width; }
    inline size_t GetHeight() const { return _height; }
    inline size_t GetLength() const { return _length; }
    inline float GetSpacing() const { return _spacing; }

    inline Vector3 GetMinCorner() const 
    {
        return Vector3Zero();
    }
    
    inline Vector3 GetMaxCorner() const 
    {
        return Vector3 { (float)_width * _spacing, (float)_height * _spacing, (float)_length * _spacing };
    }

    inline Vector3 GetCenterPos() const 
    {
        return Vector3 { (float)_width * _spacing / 2.0f, (float)_height * _spacing / 2.0f, (float)_length * _spacing / 2.0f };
    }

protected:
    inline void SetCel(int i, int j, int k, const Cel& cel) 
    {
        _grid[FlatIndex(i, j, k)] = cel;
    }

    inline Cel GetCel(int i, int j, int k) const 
    {
        return _grid[FlatIndex(i, j, k)];
    }

    inline void CopyCels(int i, int j, int k, const Grid<Cel> &src)
    {
        assert(i >= 0 && j >= 0 && k >= 0);
        int xEnd = Min(i + src._width, _width); 
        int yEnd = Min(j + src._height, _height);
        int zEnd = Min(k + src._length, _length);
        for (int z = k; z < zEnd; ++z) 
        {
            for (int y = j; y < yEnd; ++y)
            {
                size_t ourBase = FlatIndex(0, y, z);
                size_t theirBase = src.FlatIndex(0, y - j, z - k);
                for (int x = i; x < xEnd; ++x)
                {
                    const Cel &cel = src._grid[theirBase + (x - i)];
                    _grid[ourBase + x] = cel;
                }
            }
        }
    }

    inline void SubsectionCopy(int i, int j, int k, int w, int h, int l, Grid<Cel> &out) const
    {
        for (int z = k; z < k + l; ++z) 
        {
            for (int y = j; y < j + h; ++y)
            {
                size_t ourBase = FlatIndex(0, y, z);
                size_t theirBase = out.FlatIndex(0, y - j, z - k);
                for (int x = i; x < i + w; ++x)
                {
                    out._grid[theirBase + (x - i)] = _grid[ourBase + x];
                }
            }
        }
    }

    std::vector<Cel> _grid;
    size_t _width, _height, _length;
    float _spacing;
};

#endif
