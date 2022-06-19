#ifndef MAP_MAN_H
#define MAP_MAN_H

#include <deque>

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

    //Returns true if changing the size of the tile grid to the given size could not erase any existing tile data.
    inline bool ResizeSafe(int width, int height, int length)
    {
        return (width >= _tileGrid.GetWidth() && height >= _tileGrid.GetHeight() && length >= _tileGrid.GetLength());
    }

    inline void ResizeMap(int width, int height, int length, int ofsx, int ofsy, int ofsz)
    {
        _undoHistory.clear();
        _redoHistory.clear();

        // TileGrid oldGrid = _tileGrid;
        // _tileGrid = TileGrid(width, height, length, _tileGrid.GetSpacing());
        // if (!ResizeSafe(width, height, length))
        // {
        //     //Cut off the section that goes outside of the new boundaries.
        //     oldGrid = oldGrid.Subsection(
        //         ofsx < 0 ? -ofsx : 0, 
        //         ofsy < 0 ? -ofsy : 0, 
        //         ofsz < 0 ? -ofsz : 0, 
        //         oldGrid.GetWidth() + (ofsx < 0 ? ofsx : 0), 
        //         oldGrid.GetHeight() + ofsy, 
        //         oldGrid.GetLength() + ofsz);
        // }
        // _tileGrid.CopyTiles(ofsx, ofsy, ofsz, oldGrid, false);
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