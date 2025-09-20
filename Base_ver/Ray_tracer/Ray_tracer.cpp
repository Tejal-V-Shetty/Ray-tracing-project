#include <iostream>
#include <vector3.h>
#include "ray.h"

using namespace std;

//Calculates a sphere hit based on an expanded version of the formula for a sphere.
//Formula: dot(p(t)-c, p(t)-c) = R*R [Derived from x*x + y*y + z*z = R*R]
bool hit_sphere(const vector3& center, float radius, const ray& r)
{
    vector3 oc = r.origin() - center;
    float a = dot(r.direction(), r.direction());
    float b = 2.0 * dot(oc, r.direction());
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    return discriminant > 0;
}
//Background color based on Y axis distance from bottom border of the screen
vector3 color(const ray& r)
{   
    if (hit_sphere(vector3(0, 0, -1), 0.5, r))  //Sphere at Z=-1 with radius =0.5
        return vector3(1, 0, 0);
    vector3 unit_dir = unit_vector(r.direction());
    float t = 0.5 * (unit_dir.y() + 1.0);
    return (1.0 - t) * vector3(1.0, 1.0, 1.0) + t * vector3(0.5, 0.7, 1.0); //LERP between white and blue
}

int main()
{
    int nx = 200;
    int ny = 100;
    freopen("out_Ch4.ppm", "w", stdout);
    cout << "P3\n" << nx << " " << ny << "\n255\n";
    vector3 lower_left_corner(-2.0, -1.0, - 1.0);
    vector3 horizontal(4.0, 0.0, 0.0);
    vector3 vertical(0.0, 2.0, 0.0);
    vector3 origin(0.0, 0.0, 0.0);

    for (int j = ny - 1; j >= 0; j--)
    {
        for (int i = 0; i < nx; i++)
        {
            float u = float(i) / float(nx);
            float v = float(j) / float(ny);
            ray r(origin, lower_left_corner + u * horizontal + v * vertical);
            vector3 col = color(r);
            int ir = int(255.99 * col[0]);
            int ig = int(255.99 * col[1]);
            int ib = int(255.99 * col[2]);
            cout << ir << " " << ig << " " << ib << "\n";
        }
    }
}
