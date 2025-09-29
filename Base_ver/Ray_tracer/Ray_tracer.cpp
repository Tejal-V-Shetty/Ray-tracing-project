#include <iostream>
#include <vector3.h>
#include <limits>
#include <cstdlib>
#include "ray.h"
#include "hitablelist.h"
#include "sphere.h"
#include "camera.h"

using namespace std;

//For diffuse surfaces, a random point in the unit sphere drawn at the tangent to the contact point is required
vector3 random_in_unit_sphere() {
    vector3 p;
    do {
        p = 2.0 * vector3((double)rand() / RAND_MAX, (double)rand() / RAND_MAX, (double)rand() / RAND_MAX) - vector3(1, 1, 1);
    } while (p.squared_length() >= 1.0);
    return p;
}

//Background color based on Y axis distance from bottom border of the screen
vector3 color(const ray& r, hitable *world)
{   
    hit_record rec;
    if (world->hit(r, 0.001, numeric_limits<float>::max(), rec)) {
        vector3 target = rec.p + rec.normal + random_in_unit_sphere();
        return 0.5 * color( ray(rec.p, target - rec.p), world);
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
    int ns = 100;
    freopen("out_Ch7.ppm", "w", stdout);
    cout << "P3\n" << nx << " " << ny << "\n255\n";
    hitable* list[2];
    list[0] = new sphere(vector3(0, 0, -1), 0.5);
    list[1] = new sphere(vector3(0, -100.5, -1), 100);
    hitable* world = new hitable_list(list, 2);
    camera cam;
    for (int j = ny - 1; j >= 0; j--)
    {
        for (int i = 0; i < nx; i++)
        {
            vector3 col(0, 0, 0);
            for (int s = 0; s < ns; s++)
            {

                float u = float(i + ((double)rand() / RAND_MAX)) / float(nx);
                float v = float(j + ((double)rand() / RAND_MAX)) / float(ny);
                ray r = cam.get_ray(u, v);
                col += color(r, world);
            }
            
            col /= float(ns);
            col = vector3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
            int ir = int(255.99 * col[0]);
            int ig = int(255.99 * col[1]);
            int ib = int(255.99 * col[2]);
            cout << ir << " " << ig << " " << ib << "\n";
        }
    }
}
