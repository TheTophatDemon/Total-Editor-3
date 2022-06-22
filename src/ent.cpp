#include "ent.hpp"

#include "raymath.h"
#include "rlgl.h"

#include "app.hpp"
#include "draw_extras.h"
#include "text_util.hpp"
#include "assets.hpp"

void EntGrid::Draw(int fromY, int toY)
{
    for (size_t i = 0; i < _grid.size(); ++i)
    {
        if (_grid[i])
        {
            Vector3 gridCoords = UnflattenIndex(i);
            if (gridCoords.y >= (float)fromY - 0.1f && gridCoords.y <= toY + 0.1f) 
            {
                _grid[i].position = GridToWorldPos(gridCoords, true);
                _grid[i].Draw();
            }
        }
    }
}

void EntGrid::DrawLabels(Camera &camera, int fromY, int toY)
{
    if (!App::Get()->IsPreviewing())
    {
        for (size_t i = 0; i < _grid.size(); ++i)
        {
            if (_grid[i])
            {
                Vector3 gridCoords = UnflattenIndex(i);
                if (gridCoords.y >= (float)fromY - 0.1f && gridCoords.y <= toY + 0.1f) 
                {
                    if (_grid[i].properties.find("name") != _grid[i].properties.end()) 
                    {
                        //Do frustrum culling check
                        Vector3 ndc = GetWorldToNDC(_grid[i].position, camera);
                        if (ndc.z < 1.0f)
                        {
                            //Draw label
                            std::string name = _grid[i].properties.at("name");

                            float fontSize = Assets::GetFont().baseSize;
                            Vector2 projectPos = (Vector2){ (float)GetScreenWidth() * (ndc.x + 1.0f) / 2.0f, (float)GetScreenHeight() * (ndc.y + 1.0f) / 2.0f };
                            int stringWidth = GetStringWidth(Assets::GetFont(), fontSize, name);

                            float labelX = projectPos.x - (float)stringWidth / 2.0f;
                            float labelY = projectPos.y - fontSize / 2.0f;

                            DrawRectangle((int)labelX, (int)labelY, (float)stringWidth, fontSize, BLACK);
                            DrawTextEx(Assets::GetFont(), name.c_str(), (Vector2) { labelX, labelY }, fontSize, 0.0f, WHITE);
                        }
                    }
                }
            }
        }
    }
}

void Ent::Draw() const
{
    //Draw sphere
    DrawSphere(position, radius, color);
    if (!App::Get()->IsPreviewing())
    {
        //Draw axes to show orientation
        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(GetMatrix()));
        DrawAxes3D(Vector3Zero(), radius);
        rlPopMatrix();
    }
}