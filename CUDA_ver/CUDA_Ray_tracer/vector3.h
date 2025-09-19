#include<math.h>
#include<stdlib.h>

using namespace std;

class vector3 
{
public:
    vector3() {}
    vector3(float v0, float v1, float v2) 
    {
        v[0] = v0;
        v[1] = v1;
        v[2] = v2;
    }
    inline float x() const { return v[0]; }
    inline float y() const { return v[1]; }
    inline float z() const { return v[2]; }
    inline float r() const { return v[0]; }
    inline float g() const { return v[1]; }
    inline float b() const { return v[2]; }

    inline const vector3& operator+() const { return *this; }
    inline const vector3& operator-() const { return vector3(-v[0], -v[1], -v[2]); }
    inline float operator[](int i) const { return v[i]; }
    inline float& operator[](int i) { return v[i]; }

    inline vector3& operator+=(const vector3& v2);
    inline vector3& operator-=(const vector3& v2);
    inline vector3& operator*=(const vector3& v2);
    inline vector3& operator/=(const vector3& v2);
    inline vector3& operator*=(const float t);
    inline vector3& operator/=(const float t);

    inline float length() const { return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); }
    inline float squared_length() const { return v[0] * v[0] + v[1] * v[1] + v[2] * v[2]; }
    inline void make_unit_vector();


    float v[3];
};

inline istream& operator>>(istream &is, vector3& t) 
{
    is >> t.v[0] >> t.v[1] >> t.v[2];
    return is;
}

inline ostream& operator<<(ostream &os, vector3& t) 
{
    os << t.v[0] << t.v[1] << t.v[2];
    return os;
}

inline void vector3::make_unit_vector()
{
    float k = 1.0 / length();
    v[0] *= k;
    v[1] *= k;
    v[2] *= k;
}

inline vector3 operator+(const vector3& v1, const vector3& v2)
{
    return vector3(v1.v[0] + v2.v[0], v1.v[1] + v2.v[1], v1.v[2] + v2.v[2]);
}

inline vector3 operator-(const vector3& v1, const vector3& v2)
{
    return vector3(v1.v[0] - v2.v[0], v1.v[1] - v2.v[1], v1.v[2] - v2.v[2]);
}

inline vector3 operator*(const vector3& v1, const vector3& v2)
{
    return vector3(v1.v[0] * v2.v[0], v1.v[1] * v2.v[1], v1.v[2] * v2.v[2]);
}

inline vector3 operator/(const vector3& v1, const vector3& v2)
{
    return vector3(v1.v[0] / v2.v[0], v1.v[1] / v2.v[1], v1.v[2] / v2.v[2]);
}

inline vector3 operator*(float t, const vector3& v2)
{
    return vector3(t * v2.v[0], t * v2.v[1], t * v2.v[2]);
}

inline vector3 operator*( const vector3& v2, float t)
{
    return vector3(t * v2.v[0], t * v2.v[1], t * v2.v[2]);
}

inline vector3 operator/(const vector3& v2, float t)
{
    return vector3(v2.v[0] / t, v2.v[1] / t, v2.v[2] / t);
}

inline float dot(const vector3& v1, const vector3& v2)
{
    return v1.v[0] * v2.v[0] + v1.v[1] * v2.v[1] + v1.v[2] * v2.v[2];
}

inline vector3 cross(const vector3& v1, const vector3& v2)
{
    return vector3(v1.v[1] * v2.v[2]- v1.v[2] * v2.v[1],
                   -(v1.v[0] * v2.v[2] - v1.v[2] * v2.v[0]),
                   v1.v[0] * v2.v[1] - v1.v[1] * v2.v[0]);
}

inline vector3& vector3::operator+=(const vector3 &i)
{
    v[0] += i.v[0];
    v[1] += i.v[1];
    v[2] += i.v[2];
    return *this;
}

inline vector3& vector3::operator*=(const vector3& i)
{
    v[0] *= i.v[0];
    v[1] *= i.v[1];
    v[2] *= i.v[2];
    return *this;
}

inline vector3& vector3::operator/=(const vector3& i)
{
    v[0] /= i.v[0];
    v[1] /= i.v[1];
    v[2] /= i.v[2];
    return *this;
}

inline vector3& vector3::operator-=(const vector3& i)
{
    v[0] -= i.v[0];
    v[1] -= i.v[1];
    v[2] -= i.v[2];
    return *this;
}

inline vector3& vector3::operator*=(const float t)
{
    v[0] *= t;
    v[1] *= t;
    v[2] *= t;
    return *this;
}

inline vector3& vector3::operator/=(const float t) //Check later
{
    v[0] /= t;
    v[1] /= t;
    v[2] /= t;
    return *this;
}

inline vector3 unit_vector(vector3 i)
{
    return i / i.length();
}

