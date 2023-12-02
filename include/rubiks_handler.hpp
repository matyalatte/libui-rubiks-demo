#pragma once
#include <queue>
#include <random>
#include "rubiks.hpp"

namespace rubiks {

struct AnimationQueue {
    int x;
    int y;
    int z;
    int axis;  // enum Axis
    int rotation_type; // enum RotationType
    double degree_start;
    double degree_end;
    double speed;
};

// Animation handler for rubiks cube
class AnimationHandler {
 private:
    std::queue<AnimationQueue> m_animation_queues;
    double m_degree;
    RubiksCube* m_rubiks;
    bool m_is_animating;

 public:
    AnimationHandler(RubiksCube* rubiks) :
        m_degree(0), m_rubiks(rubiks), m_is_animating(false) {}

    bool IsAnimating()
    {
        return m_is_animating;
    }

    void ClearAnimations()
    {
        m_is_animating = false;
        std::queue<AnimationQueue> empty;
        std::swap(m_animation_queues, empty);
    }

    void Push(AnimationQueue q)
    {
        m_animation_queues.push(q);
    }

    // main routine for animation.
    // the return value means if it should redraw the cube or not.
    int Step()
    {
        if (m_animation_queues.size() == 0) return 0;

        // Get the current queue
        AnimationQueue &queue = m_animation_queues.front();
        int x = queue.x;
        int y = queue.y;
        int z = queue.z;
        int axis = queue.axis;
        double speed = queue.speed;
        int rotation_type = queue.rotation_type;

        if (!IsAnimating()) {
            m_degree = m_animation_queues.front().degree_start;
            m_is_animating = true;
        }

        m_degree += queue.speed;

        if ((speed > 0 && m_degree >= queue.degree_end) ||
            (speed < 0 && m_degree <= queue.degree_end) ||
            (speed == 0)) {
            // Move to the next queue
            if (rotation_type != DEGREE_0)
                m_rubiks->RotateColors(x, y, z, axis, rotation_type);
            m_rubiks->InitializeFaceRotation();
            m_animation_queues.pop();
            if (m_animation_queues.size() > 0)
                m_degree = m_animation_queues.front().degree_start;
            else
                m_is_animating = false;
        } else {
            // Rotate a face
            double th = m_degree * RUBIKS_PI / 180.0;
            m_rubiks->RotateFace(x, y, z, axis, th);
        }

        return 1;
    }
};

enum MouseState : int {
    MOUSE_STATE_IDLE = 0,
    MOUSE_STATE_ROTATE_VIEW,
    MOUSE_STATE_SELECE_AXIS,
    MOUSE_STATE_ROTATE_FACE
};

class MouseHandler {
 private:
    RubiksCube* m_rubiks;
    AnimationHandler* m_animation_handler;

    int m_state;
    Vec3D m_old_mouse_pos;

    int m_clicked_cube;  // ID of clicked cube
    int m_clicked_axis;  // vertical axis of clicked face
    Vec3D m_clicked_pos;  // clicked point on the face

    int m_rotation_axis;  //  rotation axis
    Vec3D m_rotation_center;  // intersection of rotation axis and the rotating face
    double m_rotation_theta;

 public:
    MouseHandler(RubiksCube *rubiks, AnimationHandler *handler) :
        m_rubiks(rubiks), m_animation_handler(handler)
    {
        InitializeState();
    }

    void InitializeState()
    {
        m_state = MOUSE_STATE_IDLE;
        m_clicked_cube = -1;
        m_rotation_theta = 0.0;
    }

    void Click(const Vec3D& mouse_pos)
    {
        // We don't need to use a general way to cast ray.
        // because all cube faces are parallel to axises.

        // Apply inverse rotation to ray.
        Matrix3D transposed = m_rubiks->global_rotation.Transpose();
        Vec3D ray_pos = transposed * (mouse_pos - m_rubiks->global_translation);
        Vec3D ray_vec = transposed * Vec3D(0, 0, 1);

        if (std::abs(ray_pos.x) > RUBIKS_SIZE) {
            // Check if the X faces were clicked.
            double sign_x = Sign(ray_pos.x);
            double t = (sign_x * RUBIKS_SIZE - ray_pos.x) / ray_vec.x;
            Vec3D intersection = ray_pos + ray_vec * t;
            if (std::abs(intersection.y) < RUBIKS_SIZE && std::abs(intersection.z) < RUBIKS_SIZE) {
                int x = sign_x + 1;
                int y = int((intersection.y + RUBIKS_SIZE) / CUBE_DISTANCE);
                int z = int((intersection.z + RUBIKS_SIZE) / CUBE_DISTANCE);
                m_clicked_cube = x + y * CUBE_NUM + z * CUBE_NUM * CUBE_NUM;
                m_clicked_axis = AXIS_X;
                m_clicked_pos = intersection;
                m_state = MOUSE_STATE_SELECE_AXIS;
                return;
            }
        }
        if (std::abs(ray_pos.y) > RUBIKS_SIZE) {
            // Check if the Y faces were clicked.
            double sign_y = Sign(ray_pos.y);
            double t = (sign_y * RUBIKS_SIZE - ray_pos.y) / ray_vec.y;
            Vec3D intersection = ray_pos + ray_vec * t;
            if (std::abs(intersection.x) < RUBIKS_SIZE && std::abs(intersection.z) < RUBIKS_SIZE) {
                int x = int((intersection.x + RUBIKS_SIZE) / CUBE_DISTANCE);
                int y = sign_y + 1;
                int z = int((intersection.z + RUBIKS_SIZE) / CUBE_DISTANCE);
                m_clicked_cube = x + y * CUBE_NUM + z * CUBE_NUM * CUBE_NUM;
                m_clicked_axis = AXIS_Y;
                m_clicked_pos = intersection;
                m_state = MOUSE_STATE_SELECE_AXIS;
                return;
            }
        }
        if (std::abs(ray_pos.z) > RUBIKS_SIZE) {
            // Check if the Z faces were clicked.
            double sign_z = Sign(ray_pos.z);
            double t = (sign_z * RUBIKS_SIZE - ray_pos.z) / ray_vec.z;
            Vec3D intersection = ray_pos + ray_vec * t;
            if (std::abs(intersection.x) < RUBIKS_SIZE && std::abs(intersection.y) < RUBIKS_SIZE) {
                int x = int((intersection.x + RUBIKS_SIZE) / CUBE_DISTANCE);
                int y = int((intersection.y + RUBIKS_SIZE) / CUBE_DISTANCE);
                int z = sign_z + 1;
                m_clicked_cube = x + y * CUBE_NUM + z * CUBE_NUM * CUBE_NUM;
                m_clicked_axis = AXIS_Z;
                m_clicked_pos = intersection;
                m_state = MOUSE_STATE_SELECE_AXIS;
                return;
            }
        }
        m_state = MOUSE_STATE_ROTATE_VIEW;
    }

    void RotateFace(const Vec3D& mouse_pos)
    {
        // Apply inverse rotation to ray.
        Matrix3D transposed = m_rubiks->global_rotation.Transpose();
        Vec3D ray_pos = transposed * (mouse_pos - m_rubiks->global_translation);
        Vec3D ray_vec = transposed * Vec3D(0, 0, 1);

        int x, y, z;
        CubeIdToXYZ(m_clicked_cube, &x, &y, &z, 0);

        double t;
        if (m_clicked_axis == AXIS_X)
            t = (Sign(x - 1) * RUBIKS_SIZE - ray_pos.x) / ray_vec.x;
        else if (m_clicked_axis == AXIS_Y)
            t = (Sign(y - 1) * RUBIKS_SIZE - ray_pos.y) / ray_vec.y;
        else
            t = (Sign(z - 1) * RUBIKS_SIZE - ray_pos.z) / ray_vec.z;
        Vec3D intersection = ray_pos + ray_vec * t;

        if (m_state == MOUSE_STATE_ROTATE_FACE) {
            // Rotate faces
            Vec3D cross_prod = (m_clicked_pos - m_rotation_center).Cross(intersection - m_rotation_center);
            if (m_rotation_axis == AXIS_X) {
                intersection.x = m_rotation_center.x;
                m_rotation_theta = cross_prod.x;
            } else if (m_rotation_axis == AXIS_Y) {
                intersection.y = m_rotation_center.y;
                m_rotation_theta = cross_prod.y;
            } else if (m_rotation_axis == AXIS_Z) {
                intersection.z = m_rotation_center.z;
                m_rotation_theta = cross_prod.z;
            }
            m_rotation_theta *= GROBAL_ROTATION_SPEED / RUBIKS_SIZE;
            m_rubiks->RotateFace(x, y, z, m_rotation_axis, m_rotation_theta);
        } else {
            // assert(m_state == MOUSE_STATE_SELECT_FACE)
            // Check if it should start rotation or not
            Vec3D diff = (m_clicked_pos - intersection).Abs();
            m_rotation_axis = AXIS_NONE;
            if (m_clicked_axis == AXIS_X) {
                if ((diff.y > DRAG_THRESHOLD) and (diff.y > diff.z))
                    m_rotation_axis = AXIS_Z;  // Mouse moved to y direction.
                else if (diff.z > DRAG_THRESHOLD)
                    m_rotation_axis = AXIS_Y;  // Mouse moved to z direction
            } else if (m_clicked_axis == AXIS_Y) {
                if ((diff.z > DRAG_THRESHOLD) and (diff.z > diff.x))
                    m_rotation_axis = AXIS_X;
                else if (diff.x > DRAG_THRESHOLD)
                    m_rotation_axis = AXIS_Z;
            } else if (m_clicked_axis == AXIS_Z) {
                if ((diff.x > DRAG_THRESHOLD) and (diff.x > diff.y))
                    m_rotation_axis = AXIS_Y;
                else if (diff.y > DRAG_THRESHOLD)
                    m_rotation_axis = AXIS_X;
            }

            if (m_rotation_axis == AXIS_X)
                m_rotation_center = Vec3D(intersection.x, 0.0, 0.0);
            else if (m_rotation_axis == AXIS_Y)
                m_rotation_center = Vec3D(0.0, intersection.y, 0.0);
            else if (m_rotation_axis == AXIS_Z)
                m_rotation_center = Vec3D(0.0, 0.0, intersection.z);

            if (m_rotation_axis != AXIS_NONE)
                m_state = MOUSE_STATE_ROTATE_FACE;
        }
    }

    void UnClick()
    {
        if (m_state == MOUSE_STATE_ROTATE_FACE) {
            int x, y, z;
            CubeIdToXYZ(m_clicked_cube, &x, &y, &z, 0);

            // Get the nearest angle
            m_rotation_theta *= 180 / RUBIKS_PI;
            m_rotation_theta = std::fmod(std::fmod(m_rotation_theta, 360) + 360, 360);
            int rotation_type = DEGREE_0;
            if (m_rotation_theta > 45 && m_rotation_theta <= 135)
                rotation_type = DEGREE_90;
            else if (m_rotation_theta > 135 && m_rotation_theta <= 225)
                rotation_type = DEGREE_180;
            else if (m_rotation_theta > 225 && m_rotation_theta <= 315)
                rotation_type = DEGREE_270;

            // Make an animation queue to rotate the face to the nearest angle automatically
            AnimationQueue queue;
            queue.x = x;
            queue.y = y;
            queue.z = z;
            queue.axis = m_rotation_axis;
            queue.degree_start = m_rotation_theta;
            queue.degree_end = double(rotation_type * 90);
            if (queue.degree_end - queue.degree_start > 180) {
                queue.degree_start += 360.0;
            } else if (queue.degree_start - queue.degree_end > 180) {
                queue.degree_end += 360.0;
            }
            queue.rotation_type = rotation_type;
            queue.speed = (queue.degree_end - queue.degree_start) / 5;
            if (queue.speed != 0)
                m_animation_handler->Push(queue);
        }
        InitializeState();
    }

    // main routine for mouse handler.
    // the return value means if it should redraw the cube or not.
    int Step(const Vec3D &mouse_pos, int down, int up)
    {
        if (down && m_state == MOUSE_STATE_IDLE) {
            m_old_mouse_pos = mouse_pos;
            Click(mouse_pos);
            return 1;
        } else if (up && m_state != MOUSE_STATE_IDLE) {
            UnClick();
            return 1;
        }

        if (m_state == MOUSE_STATE_IDLE)
            return 0;

        if (m_state == MOUSE_STATE_SELECE_AXIS || m_state == MOUSE_STATE_ROTATE_FACE) {
            RotateFace(mouse_pos);
            return 1;
        }

        // assert(m_state == MOUSE_STATE_ROTATE_VIEW)
        Vec3D diff = mouse_pos - m_old_mouse_pos;
        if (diff.Length() < 1)
            return 0;

        m_rubiks->GlobalRotate(diff);
        m_old_mouse_pos = mouse_pos;

        return 1;
    }
};

class Scrambler {
 private:
    std::mt19937 rng;

 public:
    Scrambler() {
        std::random_device seed_gen;
        rng = std::mt19937(seed_gen());
    }

    AnimationQueue GenerateFaceRotation() {
        // Generate a face rotation
        int axis = (rng() + 3) % 3 + 1;
        int x = 0;
        int y = 0;
        int z = 0;
        if (axis == AXIS_X)
            x = rng() % CUBE_NUM;
        else if (axis == AXIS_Y)
            y = rng() % CUBE_NUM;
        else if (axis == AXIS_Z)
            z = rng() % CUBE_NUM;
        int rotation_type = (rng() + 3) % 3 + 1;

        // Add the rotation to animation queues
        AnimationQueue queue;
        queue.x = x;
        queue.y = y;
        queue.z = z;
        queue.axis = axis;
        if (rotation_type == DEGREE_270) {
            queue.degree_start = 360.0;
            queue.speed = -15.0;
        } else {
            queue.degree_start = 0.0;
            queue.speed = 15.0;
        }
        queue.degree_end = double(rotation_type * 90);
        queue.rotation_type = rotation_type;
        return queue;
    }
};

}  // namespace rubiks
