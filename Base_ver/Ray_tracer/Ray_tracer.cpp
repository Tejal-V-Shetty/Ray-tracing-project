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

hitable* random_scene() {
    int n = 500;
    hitable** list = new hitable*[n + 1];
    list[0] = new sphere(vector3(0, -1000, 0), 1000, new lambertian(vector3(0.5, 0.5, 0.5)));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = (double)rand() / RAND_MAX;
            vector3 center(a + 0.9 * (double)rand() / RAND_MAX, 0.2, b + 0.9 * (double)rand() / RAND_MAX);
            if ((center - vector3(4, 0.2, 0)).length() > 0.9) {
                if (choose_mat < 0.8) {
                    list[i++] = new sphere(center, 0.2, new lambertian(vector3(((double)rand() / RAND_MAX)* ((double)rand() / RAND_MAX), ((double)rand() / RAND_MAX) * ((double)rand() / RAND_MAX), ((double)rand() / RAND_MAX) * ((double)rand() / RAND_MAX))));
                }
                else if (choose_mat < 0.95) {
                    list[i++] = new sphere(center,0.2,new metal(vector3(0.5*(1+ ((double)rand() / RAND_MAX)), 0.5 * (1 + ((double)rand() / RAND_MAX)), 0.5 * (1 + ((double)rand() / RAND_MAX))), 0.5 * ((double)rand() / RAND_MAX)));

                }
                else {
                    list[i++] = new sphere(center, 0.2, new dielectric(1.5));
                }
            }
        }
    }
    list[i++] = new sphere(vector3(0, 1, 0), 1.0, new dielectric(1.5));
    list[i++] = new sphere(vector3(-4, 1, 0), 1.0, new lambertian(vector3(0.4, 0.2, 0.1)));
    list[i++] = new sphere(vector3(4, 1, 0), 1.0, new metal(vector3(0.7, 0.6, 0.5), 0.0));
    
    return new hitable_list(list, i);
}

int main()
{
    int nx = 1200;
    int ny = 600;
    int ns = 100;
    freopen("out_Ch12_2.ppm", "w", stdout);
    cout << "P3\n" << nx << " " << ny << "\n255\n";
    hitable* world = random_scene();
    vector3 lookfrom(13, 3, 3);
    vector3 lookat(0, 0, 0);
    float dist_to_focus = 10.0;//(lookfrom - lookat).length();
    float aperture = 0.1;

    camera cam(lookfrom, lookat, vector3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus);
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
