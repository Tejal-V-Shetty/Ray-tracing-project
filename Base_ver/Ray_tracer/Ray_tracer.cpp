#include <iostream>
#include <vector3.h>
#include "ray.h"
#include "hitablelist.h"
#include "sphere.h"
#include <limits>

using namespace std;

//Background color based on Y axis distance from bottom border of the screen
vector3 color(const ray& r, hitable *world)
{   
    hit_record rec;
    if (world->hit(r, 0.0, numeric_limits<float>::max(), rec)) {
        return 0.5 * vector3(rec.normal.x() + 1.0, rec.normal.y() + 1.0, rec.normal.z() + 1.0);
    }
    else {
        vector3 unit_dir = unit_vector(r.direction());
        float t = 0.5 * (unit_dir.y() + 1.0);
        return (1.0 - t) * vector3(1.0, 1.0, 1.0) + t * vector3(0.5, 0.7, 1.0); //LERP between white and blue
    }
    
}

int main()
{
    int nx = 200;
    int ny = 100;
    freopen("out_Ch5.ppm", "w", stdout);
    cout << "P3\n" << nx << " " << ny << "\n255\n";
    vector3 lower_left_corner(-2.0, -1.0, - 1.0);
    vector3 horizontal(4.0, 0.0, 0.0);
    vector3 vertical(0.0, 2.0, 0.0);
    vector3 origin(0.0, 0.0, 0.0);

    hitable* list[2];
    list[0] = new sphere(vector3(0, 0, -1), 0.5);
    list[1] = new sphere(vector3(0, -100.5, -1), 100);

    hitable* world = new hitable_list(list, 2);
    for (int j = ny - 1; j >= 0; j--)
    {
        for (int i = 0; i < nx; i++)
        {
            float u = float(i) / float(nx);
            float v = float(j) / float(ny);
            ray r(origin, lower_left_corner + u * horizontal + v * vertical);
            vector3 col = color(r, world);
            int ir = int(255.99 * col[0]);
            int ig = int(255.99 * col[1]);
            int ib = int(255.99 * col[2]);
            cout << ir << " " << ig << " " << ib << "\n";
        }
    }
}
