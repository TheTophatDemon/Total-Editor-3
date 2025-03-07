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

#include "map_man.hpp"

// ======================================================================
// TILE ACTION
// ======================================================================

MapMan::TileAction::TileAction(size_t i, size_t j, size_t k, TileGrid prevState, TileGrid newState)
    : _i(i), _j(j), _k(k), 
    _prevState(prevState), 
    _newState(newState)
{}

void MapMan::TileAction::Do(MapMan& map) const
{
    map._tileGrid.CopyTiles(_i, _j, _k, _newState);
}

void MapMan::TileAction::Undo(MapMan& map) const
{
    map._tileGrid.CopyTiles(_i, _j, _k, _prevState);
}

void MapMan::ExecuteTileAction(size_t i, size_t j, size_t k, size_t w, size_t h, size_t l, Tile newTile)
{
    TileGrid prevState = _tileGrid.Subsection(i, j, k, w, h, l);
    TileGrid newState(*this, w, h, l, _tileGrid.GetSpacing(), newTile);

    _Execute(std::static_pointer_cast<Action>(
        std::make_shared<TileAction>(i, j, k, prevState, newState)
    ));
}

void MapMan::ExecuteTileAction(size_t i, size_t j, size_t k, size_t w, size_t h, size_t l, TileGrid brush)
{
    const TileGrid prevState = _tileGrid.Subsection(
            i, j, k, 
            Min(w, _tileGrid.GetWidth() - i),  //Cut off parts that go beyond map boundaries
            Min(h, _tileGrid.GetHeight() - j), 
            Min(l, _tileGrid.GetLength() - k)
        );

    TileGrid newState = prevState; //Copy the old state and merge the brush into it
    newState.CopyTiles(0, 0, 0, brush, true);

    _Execute(std::static_pointer_cast<Action>(
        std::make_shared<TileAction>(i, j, k, prevState, newState)
    ));
}

// ======================================================================
// ENT ACTION
// ======================================================================

MapMan::EntAction::EntAction(size_t i, size_t j, size_t k, bool overwrite, bool removed, Ent oldEnt, Ent newEnt)
    : _i(i), _j(j), _k(k), 
    _overwrite(overwrite), 
    _removed(removed), 
    _oldEnt(oldEnt), 
    _newEnt(newEnt)
{}

void MapMan::EntAction::Do(MapMan& map) const
{
    if (_removed)
    {
        map._entGrid.RemoveEnt(_i, _j, _k);
    }
    else
    {
        map._entGrid.AddEnt(_i, _j, _k, _newEnt);
    }
}

void MapMan::EntAction::Undo(MapMan& map) const
{
    if (_overwrite || _removed)
    {
        map._entGrid.AddEnt(_i, _j, _k, _oldEnt);
    }
    else
    {
        map._entGrid.RemoveEnt(_i, _j, _k);
    }
}

void MapMan::ExecuteEntPlacement(int i, int j, int k, Ent newEnt)
{
    Ent prevEnt = _entGrid.HasEnt(i, j, k) ? _entGrid.GetEnt(i, j, k) : Ent();
    _Execute(std::static_pointer_cast<Action>(
        std::make_shared<EntAction>(i, j, k, _entGrid.HasEnt(i, j, k), false, prevEnt, newEnt)
    ));
}

void MapMan::ExecuteEntRemoval(int i, int j, int k)
{
    _Execute(std::static_pointer_cast<Action>(
        std::make_shared<EntAction>(i, j, k, true, true, _entGrid.GetEnt(i, j, k), Ent())
    ));
}