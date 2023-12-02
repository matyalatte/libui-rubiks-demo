#pragma once
#include <array>
#include <queue>
#include "geometry.hpp"

namespace rubiks{

struct Cube : QuadModel {
    Cube() = default;

    void Initialize()
    {
        AddVertex({ -1.0,  1.0, -1.0 });
        AddVertex({ -1.0, -1.0, -1.0 });
        AddVertex({  1.0, -1.0, -1.0 });
        AddVertex({  1.0,  1.0, -1.0 });
        AddVertex({ -1.0,  1.0,  1.0 });
        AddVertex({ -1.0, -1.0,  1.0 });
        AddVertex({  1.0, -1.0,  1.0 });
        AddVertex({  1.0,  1.0,  1.0 });
        AddFace({ 0, 1, 2, 3 });  // Z-
        AddFace({ 2, 6, 7, 3 });  // X+
        AddFace({ 7, 6, 5, 4 });  // Z+
        AddFace({ 4, 5, 1, 0 });  // X-
        AddFace({ 0, 3, 7, 4 });  // Y+
        AddFace({ 1, 5, 6, 2 });  // Y-
    }

    void SetColors(std::array<uint32_t, 6> colors)
    {
        for (int i = 0; i < 6; i++)
            faces[i].color = colors[i];
    }
};

enum CubeFaceIndices : int {
    FACE_Z_MINUS = 0,
    FACE_X_PLUS,
    FACE_Z_PLUS,
    FACE_X_MINUS,
    FACE_Y_PLUS,
    FACE_Y_MINUS
};

enum Colors : uint32_t {
    COLOR_BLACK = 0x222222,
    COLOR_WHITE = 0xDDDDDD,
    COLOR_GRAY = 0xBBBBBB,
    COLOR_RED = 0xDD3333,
    COLOR_GREEN = 0x33DD33,
    COLOR_BLUE = 0x3333DD,
    COLOR_YELLOW = 0xDDDD33,
    COLOR_ORANGE = 0xDD9933
};

// This comes from Go's math.Pi, which in turn comes from http://oeis.org/A000796.
const double RUBIKS_PI = 3.14159265358979323846264338327950288419716939937510582097494459;

// Constans for game settings
const int CUBE_NUM = 3;
const double CUBE_DISTANCE = 180.0 / double(CUBE_NUM);
const double CUBE_SCALE = CUBE_DISTANCE * 0.45;
const double RUBIKS_SIZE = CUBE_DISTANCE * CUBE_NUM * 0.5;
const double DRAG_THRESHOLD = 12.0;
const double ROTATION_SPEED = RUBIKS_PI / 360;
const double GROBAL_ROTATION_SPEED = RUBIKS_PI / 360;

enum Axis : int {
    AXIS_NONE = 0,
    AXIS_X,
    AXIS_Y,
    AXIS_Z
};

enum RotaionType : int {
    DEGREE_0 = 0,
    DEGREE_90,
    DEGREE_180,
    DEGREE_270
};

double Sign(double x)
{
    return (x > 0) - (x < 0);
}

void CubeIdToXYZ(int id, int *x, int *y, int *z, int basis)
{
    *x = id % CUBE_NUM + basis;
    *y = (id / CUBE_NUM) % CUBE_NUM + basis;
    *z = id / (CUBE_NUM * CUBE_NUM) + basis;
}

struct RubiksCube {
    std::vector<Cube> cubes;
    Matrix3D global_rotation;
    Vec3D global_translation;


    RubiksCube() = default;

    void Initialize()
    {
        cubes.resize(CUBE_NUM * CUBE_NUM * CUBE_NUM);
        for (int i = 0; i < CUBE_NUM * CUBE_NUM * CUBE_NUM; i++) {
            int x, y, z;
            CubeIdToXYZ(i, &x, &y, &z, -1);
            Cube& c = cubes[i];
            c.Initialize();
            c.rotation = geometry::Identity();
            c.translation = Vec3D(x, y, z) * CUBE_DISTANCE;
            c.scale = CUBE_SCALE;
        }

        InitializeColors();
        InitializeGlobalRotation();
        global_translation = Vec3D(180.0, 180.0, RUBIKS_SIZE * 2);
    }

    void InitializeColors()
    {
        for (int i = 0; i < CUBE_NUM * CUBE_NUM * CUBE_NUM; i++) {
            std::array<uint32_t, 6> colors = {
                COLOR_GREEN, COLOR_RED, COLOR_BLUE,
                COLOR_ORANGE, COLOR_YELLOW, COLOR_WHITE };
            int x, y, z;
            CubeIdToXYZ(i, &x, &y, &z, -1);
            if (z != -1)
                colors[FACE_Z_MINUS] = COLOR_BLACK;
            if (x != 1)
                colors[FACE_X_PLUS] = COLOR_BLACK;
            if (z != 1)
                colors[FACE_Z_PLUS] = COLOR_BLACK;
            if (x != -1)
                colors[FACE_X_MINUS] = COLOR_BLACK;
            if (y != 1)
                colors[FACE_Y_PLUS] = COLOR_BLACK;
            if (y != -1)
                colors[FACE_Y_MINUS] = COLOR_BLACK;
            cubes[i].SetColors(colors);
        }
    }

    void InitializeGlobalRotation()
    {
        global_rotation = geometry::RotationX(RUBIKS_PI / 6.0) * geometry::RotationY(RUBIKS_PI / 4.0);
    }

    void InitializeFaceRotation()
    {
        for (int i = 0; i < CUBE_NUM * CUBE_NUM * CUBE_NUM; i++) {
            int x, y, z;
            CubeIdToXYZ(i, &x, &y, &z, -1);
            Cube &c = cubes[i];
            c.rotation = geometry::Identity();
            c.translation = Vec3D(x, y, z) * CUBE_DISTANCE;
        }
    }

    void GlobalRotate(Vec3D rotation)
    {
        rotation *= ROTATION_SPEED;
        global_rotation = geometry::RotationX(rotation.y) * global_rotation;
        global_rotation = geometry::RotationY(-rotation.x) * global_rotation;
    }

    void Project(std::vector<Vec3D>& projected_vertices, std::vector<Quad>& visible_faces)
    {
        // Project cubes to screen
        for (Cube& c : cubes) {
            c.Project(global_rotation, global_translation,
                      projected_vertices, visible_faces);
        }

        // Sort faces by depth in ascending order
        geometry::Zsort(visible_faces);
    }
    
    void RotateFace(int x, int y , int z, int axis, double theta)
    {
        if (axis == AXIS_X) {
            for (int y = 0; y < CUBE_NUM; y ++) {
                for (int z = 0; z < CUBE_NUM; z ++) {
                    Cube& cube = cubes[x + y * CUBE_NUM + z * CUBE_NUM * CUBE_NUM];
                    Matrix3D rotation = geometry::RotationX(theta);
                    Vec3D translation = rotation * Vec3D(0, y - 1, z - 1) + Vec3D(x - 1, 0, 0);
                    cube.rotation = rotation;
                    cube.translation = translation * CUBE_DISTANCE;
                }
            }
        } else if (axis == AXIS_Y) {
            for (int x = 0; x < CUBE_NUM; x ++) {
                for (int z = 0; z < CUBE_NUM; z ++) {
                    Cube& cube = cubes[x + y * CUBE_NUM + z * CUBE_NUM * CUBE_NUM];
                    Matrix3D rotation = geometry::RotationY(theta);
                    Vec3D translation = rotation * Vec3D(x - 1, 0, z - 1) + Vec3D(0, y - 1, 0);
                    cube.rotation = rotation;
                    cube.translation = translation * CUBE_DISTANCE;
                }
            }
        } else {
            for (int x = 0; x < CUBE_NUM; x ++) {
                for (int y = 0; y < CUBE_NUM; y ++) {
                    Cube& cube = cubes[x + y * CUBE_NUM + z * CUBE_NUM * CUBE_NUM];
                    Matrix3D rotation = geometry::RotationZ(theta);
                    Vec3D translation = rotation * Vec3D(x - 1, y - 1, 0) + Vec3D(0, 0, z - 1);
                    cube.rotation = rotation;
                    cube.translation = translation * CUBE_DISTANCE;
                }
            }
        }
    }

    void SwapFourColors90(const std::array<int, 8> &four_colors)
    {
        uint32_t c1 = cubes[four_colors[0]].faces[four_colors[1]].color;
        uint32_t c2 = cubes[four_colors[2]].faces[four_colors[3]].color;
        uint32_t c3 = cubes[four_colors[4]].faces[four_colors[5]].color;
        uint32_t c4 = cubes[four_colors[6]].faces[four_colors[7]].color;
        cubes[four_colors[0]].faces[four_colors[1]].color = c2;
        cubes[four_colors[2]].faces[four_colors[3]].color = c3;
        cubes[four_colors[4]].faces[four_colors[5]].color = c4;
        cubes[four_colors[6]].faces[four_colors[7]].color = c1;
    }

    void SwapFourColors180(const std::array<int, 8> &four_colors)
    {
        uint32_t c1 = cubes[four_colors[0]].faces[four_colors[1]].color;
        uint32_t c2 = cubes[four_colors[2]].faces[four_colors[3]].color;
        uint32_t c3 = cubes[four_colors[4]].faces[four_colors[5]].color;
        uint32_t c4 = cubes[four_colors[6]].faces[four_colors[7]].color;
        cubes[four_colors[0]].faces[four_colors[1]].color = c3;
        cubes[four_colors[2]].faces[four_colors[3]].color = c4;
        cubes[four_colors[4]].faces[four_colors[5]].color = c1;
        cubes[four_colors[6]].faces[four_colors[7]].color = c2;
    }

    void SwapFourColors270(const std::array<int, 8> &four_colors)
    {
        uint32_t c1 = cubes[four_colors[0]].faces[four_colors[1]].color;
        uint32_t c2 = cubes[four_colors[2]].faces[four_colors[3]].color;
        uint32_t c3 = cubes[four_colors[4]].faces[four_colors[5]].color;
        uint32_t c4 = cubes[four_colors[6]].faces[four_colors[7]].color;
        cubes[four_colors[0]].faces[four_colors[1]].color = c4;
        cubes[four_colors[2]].faces[four_colors[3]].color = c1;
        cubes[four_colors[4]].faces[four_colors[5]].color = c2;
        cubes[four_colors[6]].faces[four_colors[7]].color = c3;
    }

    void RotateColors(int x, int y, int z, int axis, int degree) {
        std::queue<std::array<int, 8>> swap_queues;
        if (axis == AXIS_X) {
            for (int i = 0; i < CUBE_NUM; i++) {
                swap_queues.push({
                    x + i * CUBE_NUM * CUBE_NUM, FACE_Y_MINUS,
                    x + i * CUBE_NUM + (CUBE_NUM - 1) * CUBE_NUM * CUBE_NUM, FACE_Z_PLUS,
                    x + (CUBE_NUM - 1) * CUBE_NUM + (CUBE_NUM - 1 - i) * CUBE_NUM * CUBE_NUM, FACE_Y_PLUS,
                    x + (CUBE_NUM - 1 - i) * CUBE_NUM, FACE_Z_MINUS
                });
            }
            for (int i = 0; i < CUBE_NUM - 1; i++) {
                swap_queues.push({
                    x + i * CUBE_NUM * CUBE_NUM, FACE_X_PLUS,
                    x + i * CUBE_NUM + (CUBE_NUM - 1) * CUBE_NUM * CUBE_NUM, FACE_X_PLUS,
                    x + (CUBE_NUM - 1) * CUBE_NUM + (CUBE_NUM - 1 - i) * CUBE_NUM * CUBE_NUM, FACE_X_PLUS,
                    x + (CUBE_NUM - 1 - i) * CUBE_NUM, FACE_X_PLUS
                });
                swap_queues.push({
                    x + i * CUBE_NUM * CUBE_NUM, FACE_X_MINUS,
                    x + i * CUBE_NUM + (CUBE_NUM - 1) * CUBE_NUM * CUBE_NUM, FACE_X_MINUS,
                    x + (CUBE_NUM - 1) * CUBE_NUM + (CUBE_NUM - 1 - i) * CUBE_NUM * CUBE_NUM, FACE_X_MINUS,
                    x + (CUBE_NUM - 1 - i) * CUBE_NUM, FACE_X_MINUS
                });
            }
        } else if (axis == AXIS_Y) {
            for (int i = 0; i < CUBE_NUM; i++) {
                swap_queues.push({
                    i + y * CUBE_NUM, FACE_Z_MINUS,
                    CUBE_NUM - 1 + y * CUBE_NUM + i * CUBE_NUM * CUBE_NUM, FACE_X_PLUS,
                    CUBE_NUM - 1 - i + y * CUBE_NUM + (CUBE_NUM - 1) * CUBE_NUM * CUBE_NUM, FACE_Z_PLUS,
                    y * CUBE_NUM + (CUBE_NUM - 1 - i) * CUBE_NUM * CUBE_NUM, FACE_X_MINUS
                });
            }
            for (int i = 0; i < CUBE_NUM - 1; i++) {
                swap_queues.push({
                    i + y * CUBE_NUM, FACE_Y_PLUS,
                    CUBE_NUM - 1 + y * CUBE_NUM + i * CUBE_NUM * CUBE_NUM, FACE_Y_PLUS,
                    CUBE_NUM - 1 - i + y * CUBE_NUM + (CUBE_NUM - 1) * CUBE_NUM * CUBE_NUM, FACE_Y_PLUS,
                    y * CUBE_NUM + (CUBE_NUM - 1 - i) * CUBE_NUM * CUBE_NUM, FACE_Y_PLUS
                });
                swap_queues.push({
                    i + y * CUBE_NUM, FACE_Y_MINUS,
                    CUBE_NUM - 1 + y * CUBE_NUM + i * CUBE_NUM * CUBE_NUM, FACE_Y_MINUS,
                    CUBE_NUM - 1 - i + y * CUBE_NUM + (CUBE_NUM - 1) * CUBE_NUM * CUBE_NUM, FACE_Y_MINUS,
                    y * CUBE_NUM + (CUBE_NUM - 1 - i) * CUBE_NUM * CUBE_NUM, FACE_Y_MINUS
                });
            }
        } else if (axis == AXIS_Z) {
            for (int i = 0; i < CUBE_NUM; i++) {
                swap_queues.push({
                    (CUBE_NUM - 1 - i) * CUBE_NUM + z * CUBE_NUM * CUBE_NUM, FACE_X_MINUS,
                    CUBE_NUM - 1 - i + (CUBE_NUM - 1) * CUBE_NUM + z * CUBE_NUM * CUBE_NUM, FACE_Y_PLUS,
                    CUBE_NUM - 1 + i * CUBE_NUM + z * CUBE_NUM * CUBE_NUM, FACE_X_PLUS,
                    i + z * CUBE_NUM * CUBE_NUM, FACE_Y_MINUS
                });
            }
            for (int i = 0; i < CUBE_NUM - 1; i++) {
                swap_queues.push({
                    (CUBE_NUM - 1 - i) * CUBE_NUM + z * CUBE_NUM * CUBE_NUM, FACE_Z_MINUS,
                    CUBE_NUM - 1 - i + (CUBE_NUM - 1) * CUBE_NUM + z * CUBE_NUM * CUBE_NUM, FACE_Z_MINUS,
                    CUBE_NUM - 1 + i * CUBE_NUM + z * CUBE_NUM * CUBE_NUM, FACE_Z_MINUS,
                    i + z * CUBE_NUM * CUBE_NUM, FACE_Z_MINUS
                });
                swap_queues.push({
                    (CUBE_NUM - 1 - i) * CUBE_NUM + z * CUBE_NUM * CUBE_NUM, FACE_Z_PLUS,
                    CUBE_NUM - 1 - i + (CUBE_NUM - 1) * CUBE_NUM + z * CUBE_NUM * CUBE_NUM, FACE_Z_PLUS,
                    CUBE_NUM - 1 + i * CUBE_NUM + z * CUBE_NUM * CUBE_NUM, FACE_Z_PLUS,
                    i + z * CUBE_NUM * CUBE_NUM, FACE_Z_PLUS
                });
            }
        }

        while(swap_queues.size() > 0) {
            if (degree == DEGREE_90)
                SwapFourColors90(swap_queues.front());
            else if (degree == DEGREE_180)
                SwapFourColors180(swap_queues.front());
            else if (degree == DEGREE_270)
                SwapFourColors270(swap_queues.front());
            swap_queues.pop();
        }
    }
};

}  // namespace rubiks
