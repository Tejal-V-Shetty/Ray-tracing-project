#include <iostream>
#include <vector3.h>
#include <limits>
#include <cstdlib>
#include "ray.h"
#include "hitablelist.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

using namespace std;

//Background color based on Y axis distance from bottom border of the screen
vector3 color(const ray& r, hitable *world, int depth)
{   
    hit_record rec;
    if (world->hit(r, 0.001, numeric_limits<float>::max(), rec)) {
        ray scattered;
        vector3 attenuation;
        if (depth < 50 && rec.mat->scatter(r, rec, attenuation, scattered)) {
            return attenuation * color(scattered, world, depth + 1);
        }
        else
            return vector3(0, 0, 0);
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
    freopen("out_Ch10.ppm", "w", stdout);
    cout << "P3\n" << nx << " " << ny << "\n255\n";
    hitable* list[5];
    list[0] = new sphere(vector3(0, 0, -1), 0.5, new lambertian(vector3(0.1, 0.2, 0.5)));
    list[1] = new sphere(vector3(0, -100.5, -1), 100, new lambertian(vector3(0.8, 0.8, 0.0)));
    list[2] = new sphere(vector3(1, 0, -1), 0.5, new metal(vector3(0.8, 0.6, 0.2), 0.25));
    list[3] = new sphere(vector3(-1, 0, -1), 0.5, new dielectric(1.5));
    list[4] = new sphere(vector3(-1, 0, -1), -0.45, new dielectric(1.5));
    hitable* world = new hitable_list(list, 5);
    camera cam(vector3(-2, 2, 1), vector3(0, 0, -1), vector3(0, 1, 0), 90, float(nx) / float(ny));
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
                col += color(r, world, 0);
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
