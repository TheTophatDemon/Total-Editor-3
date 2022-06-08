#include "raylib.h"
#include "raymath.h"

const float CAMERA_HEIGHT_OFFSET = 10.0f;
const float CAMERA_MOVE_SPEED_MIN = 8.0f;
const float CAMERA_MOVE_SPEED_MAX = 64.0f;
const float CAMERA_ACCELERATION = 16.0f;

void MoveCamera(Camera* camera, float mouseSensitivity) {
    static float cameraYaw = 0.0f;
    static float cameraMoveSpeed = CAMERA_MOVE_SPEED_MIN;

    //Camera controls
    Vector3 cameraMovement = Vector3Zero();
    if (IsKeyDown(KEY_D)) {
        cameraMovement.x = 1.0f;
    } else if (IsKeyDown(KEY_A)) {
        cameraMovement.x = -1.0f;
    }

    if (IsKeyDown(KEY_W)) {
        cameraMovement.z = -1.0f;
    } else if (IsKeyDown(KEY_S)) {
        cameraMovement.z = 1.0f;
    }

    if (IsKeyDown(KEY_SPACE)) {
        cameraMovement.y = 1.0f;
    } else if (IsKeyDown(KEY_LEFT_CONTROL)) {
        cameraMovement.y = -1.0f;
    }

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_A) || IsKeyDown(KEY_W) || IsKeyDown(KEY_S)) {
        cameraMoveSpeed += CAMERA_ACCELERATION * GetFrameTime();
    } else {
        cameraMoveSpeed = CAMERA_MOVE_SPEED_MIN;
    }
    cameraMoveSpeed = Clamp(cameraMoveSpeed, CAMERA_MOVE_SPEED_MIN, CAMERA_MOVE_SPEED_MAX);
    
    cameraMovement = Vector3Scale(Vector3Normalize(cameraMovement), cameraMoveSpeed * GetFrameTime());
    Vector3 rotatedMovement = Vector3Transform(cameraMovement, MatrixRotateY(cameraYaw - PI / 2.0f));
    camera->target = Vector3Add(camera->target, rotatedMovement);

    if (IsKeyPressed(KEY_R)) {
        camera->target = Vector3Zero();
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        cameraYaw += GetMouseDelta().x * mouseSensitivity * GetFrameTime();
    }

    camera->position = (Vector3){ 
        camera->target.x + cosf(cameraYaw) * CAMERA_HEIGHT_OFFSET, 
        camera->target.y + CAMERA_HEIGHT_OFFSET, 
        camera->target.z + sinf(cameraYaw) * CAMERA_HEIGHT_OFFSET };
}