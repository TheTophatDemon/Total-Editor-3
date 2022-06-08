#ifndef TILE_H
#define TILE_H

#include "raylib.h"
#include "raymath.h"

#include <stdlib.h>

typedef enum Angle { ANGLE_0, ANGLE_90, ANGLE_180, ANGLE_270 } Angle;

float AngleDegrees(Angle angle);
float AngleRadians(Angle angle);
Angle AngleBack(Angle angle);
Angle AngleForward(Angle angle);

typedef struct Tile {
    Model* shape;
    Angle angle;
    Material* material;
} Tile;

typedef struct TileGrid {
    size_t width, height, length;
    float spacing;
    Tile *grid;
} TileGrid;

TileGrid NewTileGrid(size_t width, size_t height, size_t length, float spacing);
void FreeTileGrid(TileGrid grid);

Vector3 WorldToGridPos(TileGrid* grid, Vector3 worldPos);

//Converts (whole number) grid cel coordinates to world coordinates.
//If `center` is true, then the world coordinate will be in the center of the cel instead of the corner.
Vector3 GridToWorldPos(TileGrid* grid, Vector3 gridPos, bool center);

Vector3 SnapToCelCenter(TileGrid* grid, Vector3 worldPos);

size_t FlatIndex(TileGrid* grid, int i, int j, int k);

Vector3 UnflattenIndex(TileGrid* grid, size_t idx);

void SetTile(TileGrid* grid, int i, int j, int k, Tile tile);

void UnsetTile(TileGrid* grid, int i, int j, int k);

Vector3 GetMinCorner(TileGrid* grid);

Vector3 GetMaxCorner(TileGrid* grid);

void DrawTileGrid(TileGrid* grid, Shader shader);

#endif