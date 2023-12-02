#pragma once
#include <cmath>
#include <algorithm>
#include <iterator>
#include <vector>

struct Vec3D {
    double x;
    double y;
    double z;

    Vec3D() = default;

    Vec3D(double _x, double _y, double _z)
        : x(_x)
        , y(_y)
        , z(_z) {}

    Vec3D(int _x, int _y, int _z)
        : x(double(_x))
        , y(double(_y))
        , z(double(_z)) {}

    double Dot(const Vec3D& v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }

    double Length()
    {
        return std::sqrt(Dot(*this));
    }

    Vec3D Normalized()
    {
        return *this / Length();
    }

    Vec3D Cross(const Vec3D& v) const
    {
        double newx = y * v.z - z * v.y;
        double newy = z * v.x - x * v.z;
        double newz = x * v.y - y * v.x;
        return { newx, newy, newz };
    }

    Vec3D Abs() const
    {
        return { std::abs(x), std::abs(y), std::abs(z) };
    }

    Vec3D operator+(const Vec3D& v) const
    {
        return { x + v.x, y + v.y , z + v.z };
    }

    Vec3D operator-() const
    {
        return { -x, -y, -z };
    }

    Vec3D operator-(const Vec3D& v) const
    {
        return { x - v.x, y - v.y, z - v.z };
    }

    Vec3D operator*(double s) const
    {
        return { x * s, y * s, z * s };
    }

    Vec3D operator/(double s) const
    {
        return { x / s, y / s, z / s };
    }

    Vec3D& operator +=(const Vec3D& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vec3D& operator -=(const Vec3D& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    Vec3D& operator *=(double s)
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    Vec3D& operator /=(double s)
    {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    Vec3D& operator =(const Vec3D& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }
};

struct Matrix3D {
    double m11, m12, m13;
    double m21, m22, m23;
    double m31, m32, m33;

    Matrix3D() = default;

    Matrix3D Transpose() const
    {
        return {
            m11, m21, m31,
            m12, m22, m32,
            m13, m23, m33
        };
    }

    Matrix3D operator*(double s) const
    {
        return {
            m11 * s, m12 * s, m13 * s,
            m21 * s, m22 * s, m23 * s,
            m31 * s, m32 * s, m33 * s
        };
    }

    Matrix3D& operator*=(double s)
    {
        m11 *= s;
        m12 *= s;
        m13 *= s;
        m21 *= s;
        m22 *= s;
        m23 *= s;
        m31 *= s;
        m32 *= s;
        m33 *= s;
        return *this;
    }

    Matrix3D operator*(const Matrix3D& A) const
    {
        double new11 = m11 * A.m11 + m12 * A.m21 + m13 * A.m31;
        double new12 = m11 * A.m12 + m12 * A.m22 + m13 * A.m32;
        double new13 = m11 * A.m13 + m12 * A.m23 + m13 * A.m33;
        double new21 = m21 * A.m11 + m22 * A.m21 + m23 * A.m31;
        double new22 = m21 * A.m12 + m22 * A.m22 + m23 * A.m32;
        double new23 = m21 * A.m13 + m22 * A.m23 + m23 * A.m33;
        double new31 = m31 * A.m11 + m32 * A.m21 + m33 * A.m31;
        double new32 = m31 * A.m12 + m32 * A.m22 + m33 * A.m32;
        double new33 = m31 * A.m13 + m32 * A.m23 + m33 * A.m33;
        return {
            new11, new12, new13,
            new21, new22, new23,
            new31, new32, new33
        };
    }

    Matrix3D& operator*=(const Matrix3D& A)
    {
        double new11 = m11 * A.m11 + m12 * A.m21 + m13 * A.m31;
        double new12 = m11 * A.m12 + m12 * A.m22 + m13 * A.m32;
        double new13 = m11 * A.m13 + m12 * A.m23 + m13 * A.m33;
        double new21 = m21 * A.m11 + m22 * A.m21 + m23 * A.m31;
        double new22 = m21 * A.m12 + m22 * A.m22 + m23 * A.m32;
        double new23 = m21 * A.m13 + m22 * A.m23 + m23 * A.m33;
        double new31 = m31 * A.m11 + m32 * A.m21 + m33 * A.m31;
        double new32 = m31 * A.m12 + m32 * A.m22 + m33 * A.m32;
        double new33 = m31 * A.m13 + m32 * A.m23 + m33 * A.m33;
        m11 = new11;
        m12 = new12;
        m13 = new13;
        m21 = new21;
        m22 = new22;
        m23 = new23;
        m31 = new31;
        m32 = new32;
        m33 = new33;
        return *this;
    }

    Vec3D operator*(const Vec3D& v) const
    {
        double x = m11 * v.x + m12 * v.y + m13 * v.z;
        double y = m21 * v.x + m22 * v.y + m23 * v.z;
        double z = m31 * v.x + m32 * v.y + m33 * v.z;
        return { x, y, z };
    }

    Matrix3D& operator=(const Matrix3D& A)
    {
        m11 = A.m11;
        m12 = A.m12;
        m13 = A.m13;
        m21 = A.m21;
        m22 = A.m22;
        m23 = A.m23;
        m31 = A.m31;
        m32 = A.m32;
        m33 = A.m33;
        return *this;
    }
};

struct Quad {
    // v1 - v4
    // |    |
    // v2 - v3
    int v1;
    int v2;
    int v3;
    int v4;
    uint32_t color;
    double z;  // it can store depth for z sorting

    void IncIndices(int inc)
    {
        v1 += inc;
        v2 += inc;
        v3 += inc;
        v4 += inc;
    }
};

struct QuadModel {
    std::vector<Vec3D> vertices;
    std::vector<Quad> faces;
    Matrix3D rotation;
    Vec3D translation;
    double scale;

    QuadModel() = default;

    void AddVertex(const Vec3D& v)
    {
        vertices.push_back(v);
    }

    void AddFace(const Quad& f)
    {
        faces.push_back(f);
    }

    void Project(
        const Matrix3D& global_rotation,
        const Vec3D& global_translation,
        std::vector<Vec3D>& projected_vertices,
        std::vector<Quad>& visible_faces)
    {
        // Calculate projected coordinates
        std::vector<Vec3D> new_projected_vertices;
        for (const Vec3D& v : vertices) {
            Vec3D local_vec = rotation * v * scale + translation;
            Vec3D global_vec = global_rotation * local_vec + global_translation;
            new_projected_vertices.push_back(global_vec);
        }

        // Collect visible faces
        std::vector<Quad> new_visible_faces;
        for (Quad& f : faces) {
            Vec3D v1 = new_projected_vertices[f.v1];
            Vec3D v2 = new_projected_vertices[f.v2];
            Vec3D v3 = new_projected_vertices[f.v3];
            Vec3D cross_prod = (v2 - v1).Cross(v3 - v2);
            if (cross_prod.z <= 0) {
                // invisible
                continue;
            }
            Vec3D v4 = new_projected_vertices[f.v4];
            Quad copied_f = f;
            copied_f.z = (v1.z + v2.z + v3.z + v4.z) / 4;  // store center point for z sorting
            copied_f.IncIndices(projected_vertices.size());
            new_visible_faces.push_back(copied_f);
        }

        // Concatnate vectors
        std::copy(new_projected_vertices.begin(),
                  new_projected_vertices.end(),
                  std::back_inserter(projected_vertices));
        std::copy(new_visible_faces.begin(),
                  new_visible_faces.end(),
                  std::back_inserter(visible_faces));
    }
};

namespace geometry {

const Matrix3D Zero()
{
    return {
        0, 0, 0,
        0, 0, 0,
        0, 0, 0
    };
}

const Matrix3D Identity()
{
    return {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    };
}

const Matrix3D RotationX(double theta)
{
    double s = std::sin(theta);
    double c = std::cos(theta);
    return {
        1, 0,  0,
        0, c, -s,
        0, s,  c
    };
}

const Matrix3D RotationY(double theta)
{
    double s = std::sin(theta);
    double c = std::cos(theta);
    return {
        c, 0, s,
        0, 1, 0,
        -s, 0, c
    };
}

const Matrix3D RotationZ(double theta)
{
    double s = std::sin(theta);
    double c = std::cos(theta);
    return {
        c, -s, 0,
        s,  c, 0,
        0,  0, 1
    };
}

static bool CompDepth(const Quad& q1, const Quad& q2)
{
    return (q1.z > q2.z);
}

void Zsort(std::vector<Quad>& visible_faces)
{
    // Sort faces by depth in ascending order
    std::sort(visible_faces.begin(), visible_faces.end(), CompDepth);
}

}  // namespace geometry
