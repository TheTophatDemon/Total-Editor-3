#ifndef MAP_MAN_H
#define MAP_MAN_H

#include <deque>
#include <cstdint>

#include "tile.hpp"

//It's either a class that manages map data modification, saving/loading, and undo/redo operations, or a lame new Megaman boss.
class MapMan
{
public:
    struct TileAction 
    {
        size_t i, j, k;
        TileGrid prevState;
        TileGrid newState;
    };

    inline void NewMap(int width, int height, int length) 
    {
        _tileGrid = TileGrid(width, height, length, _tileGrid.GetSpacing());
        _undoHistory.clear();
        _redoHistory.clear();
    }

    inline const TileGrid& Map() const { return _tileGrid; }

    inline void DrawMap(Vector3 pos, int fromY, int toY) 
    {
        _tileGrid.Draw(pos, fromY, toY);
    }

    //Regenerates the map, extending one of the grid's dimensions on the given axis. Returns false if the change would result in an invalid map size.
    inline void ExpandMap(Direction axis, int amount)
    {
        int newWidth  = _tileGrid.GetWidth();
        int newHeight = _tileGrid.GetHeight();
        int newLength = _tileGrid.GetLength();
        int ofsx, ofsy, ofsz;
        ofsx = ofsy = ofsz = 0;

        switch (axis)
        {
            case Direction::Z_NEG: newLength += amount; ofsz += amount; break;
            case Direction::Z_POS: newLength += amount; break;
            case Direction::X_NEG: newWidth += amount; ofsx += amount; break;
            case Direction::X_POS: newWidth += amount; break;
            case Direction::Y_NEG: newHeight += amount; ofsy += amount; break;
            case Direction::Y_POS: newHeight += amount; break;
        }

        _undoHistory.clear();
        _redoHistory.clear();
        TileGrid oldGrid = _tileGrid;
        _tileGrid = TileGrid(newWidth, newHeight, newLength, _tileGrid.GetSpacing());       
        _tileGrid.CopyTiles(ofsx, ofsy, ofsz, oldGrid, false);
    }

    //Reduces the size of the grid until it fits perfectly around all the non-empty tiles in the map.
    inline void ShrinkMap()
    {
        size_t minX, minY, minZ;
        size_t maxX, maxY, maxZ;
        minX = minY = minZ = UINT64_MAX;
        maxX = maxY = maxZ = 0;
        for (size_t x = 0; x < _tileGrid.GetWidth(); ++x)
        {
            for (size_t y = 0; y < _tileGrid.GetHeight(); ++y)
            {
                for (size_t z = 0; z < _tileGrid.GetLength(); ++z)
                {
                    if (_tileGrid.GetTile(x, y, z).shape != nullptr)
                    {
                        if (x < minX) minX = x;
                        if (y < minY) minY = y;
                        if (z < minZ) minZ = z;
                        if (x > maxX) maxX = x;
                        if (y > maxY) maxY = y;
                        if (z > maxZ) maxZ = z;
                    }
                }
            }
        }
        if (minX > maxX || minY > maxY || minZ > maxZ)
        {
            //If there aren't any tiles, just make it 1x1x1.
            _tileGrid = TileGrid(1, 1, 1, _tileGrid.GetSpacing());
        }
        else
        {
            _tileGrid = _tileGrid.Subsection(minX, minY, minZ, maxX - minX + 1, maxY - minY + 1, maxZ - minZ + 1);
        }
    }

    //Executes a tile action for filling an area with one tile
    TileAction &ExecuteTileAction(size_t i, size_t j, size_t k, size_t w, size_t h, size_t l, Tile newTile);
    //Executes a tile action for filling an area using a brush
    TileAction &ExecuteTileAction(size_t i, size_t j, size_t k, size_t w, size_t h, size_t l, TileGrid brush);

    inline void Undo()
    {
        if (!_undoHistory.empty())
        {
            TileAction &action = _undoHistory.back();
            _UndoAction(action);
            _redoHistory.push_back(action);
            _undoHistory.pop_back();
        }
    }

    inline void Redo()
    {
        if (!_redoHistory.empty())
        {
            //Redo (Ctrl + Y)
            TileAction &action = _redoHistory.back();
            _DoAction(action);
            _undoHistory.push_back(action);
            _redoHistory.pop_back();
        }
    }
private:
    void _DoAction(TileAction &action);
    void _UndoAction(TileAction &action);

    TileGrid _tileGrid;

    //Stores recently executed actions to be undone on command.
    std::deque<TileAction> _undoHistory;
    //Stores recently undone actions to be redone on command, unless the history is altered.
    std::deque<TileAction> _redoHistory;
};

#endif