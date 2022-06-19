#include "map_man.hpp"

#include "app.hpp"

MapMan::TileAction &MapMan::ExecuteTileAction(size_t i, size_t j, size_t k, size_t w, size_t h, size_t l, Tile newTile)
{
    _undoHistory.push_back((TileAction){
        .i = i, 
        .j = j, 
        .k = k, 
        .prevState = _tileGrid.Subsection(i, j, k, w, h, l),
        .newState = TileGrid(w, h, l, _tileGrid.GetSpacing(), newTile) 
    });
    if (_undoHistory.size() > App::Get()->GetUndoMax()) _undoHistory.pop_front();
    _redoHistory.clear();

    _DoAction(_undoHistory.back());
    return _undoHistory.back();
}

MapMan::TileAction &MapMan::ExecuteTileAction(size_t i, size_t j, size_t k, size_t w, size_t h, size_t l, TileGrid brush)
{
    const TileGrid prevState = _tileGrid.Subsection(
            i, j, k, 
            Min(w, _tileGrid.GetWidth() - i),  //Cut off parts that go beyond map boundaries
            Min(h, _tileGrid.GetHeight() - j), 
            Min(l, _tileGrid.GetLength() - k)
        );

    TileGrid newState = prevState; //Copy the old state and merge the brush into it
    newState.CopyTiles(0, 0, 0, brush, true);

    _undoHistory.push_back((TileAction){
        .i = i, 
        .j = j, 
        .k = k, 
        .prevState = prevState,
        .newState = newState
    });
    if (_undoHistory.size() > App::Get()->GetUndoMax()) _undoHistory.pop_front();
    _redoHistory.clear();

    _DoAction(_undoHistory.back());
    return _undoHistory.back();
}

void MapMan::_DoAction(TileAction &action)
{
    _tileGrid.CopyTiles(action.i, action.j, action.k, action.newState);
}

void MapMan::_UndoAction(TileAction &action)
{
    _tileGrid.CopyTiles(action.i, action.j, action.k, action.prevState);
}