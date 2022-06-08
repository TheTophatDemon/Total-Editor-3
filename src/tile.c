#include "tile.h"
#include "array_list.h"

#include <string.h>

extern ModelList gShapes;
extern size_t gShapesMeshCount;
extern MaterialList gMaterialsInstanced;

float AngleDegrees(Angle angle) {
    return (float)(((int)angle) * 90);
}

float AngleRadians(Angle angle) {
    return (float)angle * PI / 2.0f;
}

Angle AngleBack(Angle angle) {
    return (Angle)((angle - 1) < ANGLE_0 ? ANGLE_270 : (angle - 1));
}

Angle AngleForward(Angle angle) {
    return (Angle)((angle + 1) > ANGLE_270 ? ANGLE_0 : (angle + 1));
}

Vector3 WorldToGridPos(TileGrid* grid, Vector3 worldPos) {
    return (Vector3){ floorf(worldPos.x / grid->spacing) + (grid->width / 2), floorf(worldPos.y / grid->spacing) + (grid->height / 2), floorf(worldPos.z / grid->spacing) + (grid->length / 2) };
}

TileGrid NewTileGrid(size_t width, size_t height, size_t length, float spacing) {
    return (TileGrid){
        .width=width,
        .height=height,
        .length=length,
        .spacing=spacing,
        .grid=RL_CALLOC(width*height*length, sizeof(Tile))
    };
}

void FreeTileGrid(TileGrid grid) {
    RL_FREE(grid.grid);
}

Vector3 GridToWorldPos(TileGrid* grid, Vector3 gridPos, bool center) {
    if (center) {
        return (Vector3){
            (gridPos.x * grid->spacing) + (grid->spacing / 2.0f) - (grid->width * grid->spacing / 2.0f),
            (gridPos.y * grid->spacing) + (grid->spacing / 2.0f) - (grid->height * grid->spacing / 2.0f),
            (gridPos.z * grid->spacing) + (grid->spacing / 2.0f) - (grid->length * grid->spacing / 2.0f),
        };
    } else {
        return (Vector3){
            (gridPos.x * grid->spacing) - (grid->width * grid->spacing / 2.0f),
            (gridPos.y * grid->spacing) - (grid->height * grid->spacing / 2.0f),
            (gridPos.z * grid->spacing) - (grid->length * grid->spacing / 2.0f),
        };
    }
}

Vector3 SnapToCelCenter(TileGrid* grid, Vector3 worldPos) {
    worldPos.x = (floorf(worldPos.x / grid->spacing) * grid->spacing) + (grid->spacing / 2.0f);
    worldPos.y = (floorf(worldPos.y / grid->spacing) * grid->spacing) + (grid->spacing / 2.0f);
    worldPos.z = (floorf(worldPos.z / grid->spacing) * grid->spacing) + (grid->spacing / 2.0f);
    return worldPos;
}

size_t FlatIndex(TileGrid* grid, int i, int j, int k) {
    return i + (k * grid->width) + (j * grid->width * grid->length);
}

void SetTile(TileGrid* grid, int i, int j, int k, Tile tile) {
    grid->grid[FlatIndex(grid, i, j, k)] = tile;
}

void UnsetTile(TileGrid* grid, int i, int j, int k) {
    grid->grid[FlatIndex(grid, i, j, k)].shape = NULL;
}

Vector3 UnflattenIndex(TileGrid* grid, size_t idx) {
    return (Vector3){
        (float)(idx % grid->width),
        (float)(idx / (grid->width * grid->length)),
        (float)((idx / grid->width) % grid->length)
    };
}

Vector3 GetMinCorner(TileGrid* grid) {
    return (Vector3){ -((int)grid->width / 2) * grid->spacing, -((int)grid->height / 2) * grid->spacing, -((int)grid->length / 2) * grid->spacing };
}

Vector3 GetMaxCorner(TileGrid* grid) {
    return (Vector3){ +((int)grid->width / 2) * grid->spacing, +((int)grid->height / 2) * grid->spacing, +((int)grid->length / 2) * grid->spacing };
}

void DrawTileGrid(TileGrid* grid, Shader shader) {

    typedef struct Bucket {
        MatrixList matrices;
        Material *material;
        Mesh *mesh;
    } Bucket;

    size_t numBuckets = gShapesMeshCount * gMaterialsInstanced.length;
    Bucket *buckets = RL_CALLOC(numBuckets, sizeof(Bucket));
    memset(buckets, 0, sizeof(Bucket) * numBuckets);

    for (size_t t = 0; t < grid->width * grid->height * grid->length; ++t) {
        if (grid->grid[t].material && grid->grid[t].shape) {
            //Calculate world space matrix for the tile
            Vector3 worldPos = GridToWorldPos(grid, UnflattenIndex(grid, t), true);
            Matrix matrix = MatrixMultiply(
                MatrixRotateY(AngleRadians(grid->grid[t].angle)), 
                MatrixTranslate(worldPos.x, worldPos.y, worldPos.z));
            for (size_t m = 0; m < grid->grid[t].shape->meshCount; ++m) {
                //Add the tile's transform to the instance arrays for each mesh
                size_t b = 0;
                for (b = 0; b < numBuckets; ++b) {
                    if (buckets[b].material == grid->grid[t].material && buckets[b].mesh == &grid->grid[t].shape->meshes[m]) {
                        LIST_APPEND(Matrix, buckets[b].matrices, matrix);
                        break;
                    }
                }
                if (b == numBuckets) {
                    //Allocate a new bucket for this combination of mesh and material.
                    for (b = 0; b < numBuckets; ++b) {
                        if (buckets[b].material == NULL || buckets[b].mesh == NULL) {
                            buckets[b].material = grid->grid[t].material;
                            buckets[b].mesh = &grid->grid[t].shape->meshes[m];
                            LIST_APPEND(Matrix, buckets[b].matrices, matrix);
                        }
                    }
                }
            }
        }
    }
    
    //Render the groups of tiles for each combination of shape and material.
    for (size_t b = 0; b < numBuckets; ++b) {
        if (buckets[b].material && buckets[b].mesh) {
            DrawMeshInstanced(*buckets[b].mesh, *buckets[b].material, buckets[b].matrices.data, buckets[b].matrices.length);

            free(buckets[b].matrices.data);
        }
    }

    RL_FREE(buckets);

    //Create a hash map of dynamic arrays for each combination of material and mesh
    // auto groups = std::map<std::pair<Material*, Mesh*>, std::vector<Matrix>>();
    // for (size_t t = 0; t < _grid.size(); ++t) {
    //     const Tile& tile = _grid[t];
    //     if (tile.material && tile.shape) {
    //         //Calculate world space matrix for the tile
    //         Vector3 worldPos = GridToWorldPos(UnflattenIndex(t), true);
    //         Matrix matrix = MatrixMultiply(
    //             MatrixRotateY(AngleRadians(tile.angle)), 
    //             MatrixTranslate(worldPos.x, worldPos.y, worldPos.z));

    //         for (size_t m = 0; m < tile.shape->meshCount; ++m) {
    //             //Add the tile's transform to the instance arrays for each mesh
    //             auto pair = std::make_pair(tile.material, &tile.shape->meshes[m]);
    //             if (groups.find(pair) == groups.end()) {
    //                 //Put in a vector for this pair if there hasn't been one already
    //                 groups[pair] = std::vector<Matrix>();
    //             }
    //             groups[pair].push_back(matrix);
    //         }
    //     }
    // }

    // //This allows us to call DrawMeshInstanced for each combination of material and mesh.
    // for (auto& [pair, matrices] : groups) {
    //     DrawMeshInstanced(*pair.second, *pair.first, matrices.data(), matrices.size());
    // }
}