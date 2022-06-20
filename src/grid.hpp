#ifndef GRID_H
#define GRID_H

#include <stdlib.h>
#include <vector>
#include <assert.h>

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
            return (Vector3) {
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
        return (Vector3) { (float)_width * _spacing, (float)_height * _spacing, (float)_length * _spacing };
    }

    inline Vector3 GetCenterPos() const 
    {
        return (Vector3) { (float)_width * _spacing / 2.0f, (float)_height * _spacing / 2.0f, (float)_length * _spacing / 2.0f };
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

    std::vector<Cel> _grid;
    size_t _width, _height, _length;
    float _spacing;
};

#endif