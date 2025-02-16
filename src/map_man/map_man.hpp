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

#ifndef MAP_MAN_H
#define MAP_MAN_H

#include <deque>
#include <cstdint>
#include <memory>
#include <stdint.h>
#include <limits>
#include <filesystem>
namespace fs = std::filesystem;

#include "../tile.hpp"
#include "../ent.hpp"

//It's either a class that manages map data modification, saving/loading, and undo/redo operations, or a lame new Megaman boss.
class MapMan
{
public:
    class Action 
    {
    public:
        virtual void Do(MapMan &map) const = 0;
        virtual void Undo(MapMan &map) const = 0;
    };

    class TileAction : public Action
    {
    public:
        TileAction(size_t i, size_t j, size_t k, TileGrid prevState, TileGrid newState);
        
        virtual void Do(MapMan& map) const override;
        virtual void Undo(MapMan& map) const override;

        size_t _i, _j, _k;
        TileGrid _prevState;
        TileGrid _newState;
    };

    class EntAction : public Action
    {
    public:
        EntAction(size_t i, size_t j, size_t k, bool overwrite, bool removed, Ent oldEnt, Ent newEnt);

        virtual void Do(MapMan &map) const override;
        virtual void Undo(MapMan &map) const override;
    protected:
        size_t _i, _j, _k;
        bool _overwrite; //Indicates if there was an entity underneath the one placed that must be restored when undoing.
        bool _removed; //Indicates if the new cel value is empty.
        Ent  _oldEnt;
        Ent  _newEnt;
    };

    MapMan();

    void NewMap(int width, int height, int length);

    inline const TileGrid& Tiles() const { return _tileGrid; }
    inline const EntGrid& Ents() const { return _entGrid; }

    void DrawMap(Camera &camera, int fromY, int toY);
    void Draw2DElements(Camera &camera, int fromY, int toY);

    //Regenerates the map, extending one of the grid's dimensions on the given axis. Returns false if the change would result in an invalid map size.
    void ExpandMap(Direction axis, int amount);

    //Reduces the size of the grid until it fits perfectly around all the non-empty cels in the map.
    void ShrinkMap();

    //Saves the map as a .te3 file at the given path. Returns false if there was an error.
    bool SaveTE3Map(fs::path filePath);

    //Loads a .te3 map from the given path. Returns false if there was an error.
    bool LoadTE3Map(fs::path filePath);

    //Loads and converts a Total Invasion II .ti map from the given path. Returns false on error.
    bool LoadTE2Map(fs::path filePath);

    //Exports the map as a .gltf file, returning false on error.
    //If separateGeometry is true, then the geometry will be put into separate
    //GLTF nodes according to their tile texture.
    bool ExportGLTFScene(fs::path filePath, bool separateGeometry);

    //Executes a undoable tile action for filling an area with one tile
    void ExecuteTileAction(size_t i, size_t j, size_t k, size_t w, size_t h, size_t l, Tile newTile);
    //Executes a undoable tile action for filling an area using a brush
    void ExecuteTileAction(size_t i, size_t j, size_t k, size_t w, size_t h, size_t l, TileGrid brush);
    //Executes an undoable entity action for placing an entity
    void ExecuteEntPlacement(int i, int j, int k, Ent newEnt);
    //Executes an undoable entity action for removing an entity.
    void ExecuteEntRemoval(int i, int j, int k);

    TexID GetOrAddTexID(const fs::path &texturePath);
    ModelID GetOrAddModelID(const fs::path &modelPath);

    fs::path PathFromTexID(const TexID id) const;
    fs::path PathFromModelID(const ModelID id) const;
    Model ModelFromID(const ModelID id) const;
    Texture TexFromID(const TexID id) const;

    inline const std::vector<std::shared_ptr<Assets::ModelHandle>> GetModelList() const { return _modelList; }
    const std::vector<fs::path> GetModelPathList() const;
    inline int GetNumModels() const { return _modelList.size(); }
    inline const std::vector<std::shared_ptr<Assets::TexHandle>> GetTextureList() const { return _textureList; }
    const std::vector<fs::path> GetTexturePathList() const;
    inline int GetNumTextures() const { return _textureList.size(); }

    inline Vector3 GetDefaultCameraPosition() const { return _defaultCameraPosition; }
    inline void SetDefaultCameraPosition(Vector3 pos) { _defaultCameraPosition = pos; }
    inline Vector3 GetDefaultCameraAngles() const { return _defaultCameraAngles; }
    inline void SetDefaultCameraAngles(Vector3 angles) { _defaultCameraAngles = angles; }

    void Undo();
    void Redo();
private:
    void _Execute(std::shared_ptr<Action> action);

    TileGrid _tileGrid;
    EntGrid _entGrid;

    Vector3 _defaultCameraPosition, _defaultCameraAngles;

    std::vector<std::shared_ptr<Assets::TexHandle>> _textureList;
    std::vector<std::shared_ptr<Assets::ModelHandle>> _modelList;

    //Stores recently executed actions to be undone on command.
    std::deque<std::shared_ptr<Action>> _undoHistory;
    //Stores recently undone actions to be redone on command, unless the history is altered.
    std::deque<std::shared_ptr<Action>> _redoHistory;
};

#endif
