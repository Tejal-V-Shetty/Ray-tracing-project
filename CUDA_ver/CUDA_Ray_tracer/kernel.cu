
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <iostream>
#include <limits>
#include "vector3.h"
#include "ray.h"
#include "hitablelist.h"
#include "sphere.h"

using namespace std;

#define checkCudaErrors(val) check_cuda((val), #val, __FILE__, __LINE__)
void check_cuda(cudaError_t result, char const* const func, const char* const file, int const line)
{
	if (result)
	{
		cerr << "CUDA error = " << static_cast<unsigned int>(result) << "at" << file << ":" << line << " " << func << "' \n";
		cudaDeviceReset();
		exit(99);
	}
}

__global__ void create_world(hitable **d_list, hitable **d_world)
{
    if (threadIdx.x == 0 && blockIdx.x == 0)    //Only initialize them once
    {
        *(d_list) = new sphere(vector3(0, 0, -1), 0.5);
        *(d_list + 1) = new sphere(vector3(0, -100.5, -1), 100);
        *d_world = new hitable_list(d_list, 2);
    }
}

__global__ void free_world(hitable** d_list, hitable** d_world)
{
    delete* (d_list);
    delete* (d_list + 1);
    delete* d_world;
}

//Calculates a sphere hit based on an expanded version of the formula for a sphere.
//Formula: dot(p(t)-c, p(t)-c) = R*R [Derived from x*x + y*y + z*z = R*R]
__device__ bool hit_sphere(const vector3& center, float radius, const ray& r)
{
    vector3 oc = r.origin() - center;
    float a = dot(r.direction(), r.direction());
    float b = 2.0f * dot(oc, r.direction());
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4.0f * a * c;
    return discriminant > 0;
}

__device__ vector3 color(const ray& r, hitable **world)
{
    hit_record rec;
    if ((*world)->hit(r, 0.0f, FLT_MAX, rec)) {
        return 0.5f * vector3(rec.normal.x() + 1.0f, rec.normal.y() + 1.0f, rec.normal.z() + 1.0f);
    }
    else {
        vector3 unit_dir = unit_vector(r.direction());
        float t = 0.5f * (unit_dir.y() + 1.0f);
        return (1.0f - t) * vector3(1.0, 1.0, 1.0) + t * vector3(0.5, 0.7, 1.0); //LERP between white and blue
    }
}

__global__ void render(vector3* fb, int max_x, int max_y, vector3 lower_left_corner, vector3 horizontal, vector3 vertical, vector3 origin, hitable **world)
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;
    if ((i >= max_x) || (j >= max_y))
        return;
    int pixel_index = j * max_x + i;
    float u = float(i) / float(max_x);
    float v = float(j) / float(max_y);
    ray r(origin, lower_left_corner + u * horizontal + v * vertical);
    fb[pixel_index] = color(r, world);
}

int main()
{
    int nx = 200;
    int ny = 100;
    int num_pixels = nx * ny;
    size_t fb_size = 3 * num_pixels * sizeof(float);

    //Frame buffer allocation
    vector3* fb;
    checkCudaErrors(cudaMallocManaged((void**) & fb, fb_size));

    vector3 lower_left_corner(-2.0, -1.0, -1.0);
    vector3 horizontal(4.0, 0.0, 0.0);
    vector3 vertical(0.0, 2.0, 0.0);
    vector3 origin(0.0, 0.0, 0.0);

    hitable** d_list;
    checkCudaErrors(cudaMalloc((void**)&d_list, 2 * sizeof(hitable*)));
    hitable** d_world;
    checkCudaErrors(cudaMalloc((void**)&d_world, sizeof(hitable *)));
    create_world<<<1,1>>>(d_list, d_world);

    int tx = 8, ty = 8; //Threads
    dim3 blocks(nx / tx + 1, ny / ty + 1);
    dim3 threads(tx, ty);
    render<<<blocks, threads >>> (fb, nx, ny, lower_left_corner, horizontal, vertical, origin, d_world);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    //Output the image
    freopen("out_Ch5.ppm", "w", stdout);
    cout << "P3\n" << nx << " " << ny << "\n255\n";

    for (int j = ny - 1; j >= 0; j--)
    {
        for (int i = 0; i < nx; i++)
        {
            size_t pixel_index = j * nx + i ;
            vector3 col(fb[pixel_index]);
            int ir = int(255.99 * col.r());
            int ig = int(255.99 * col.g());
            int ib = int(255.99 * col.b());
            cout << ir << " " << ig << " " << ib << "\n";
        }
    }
    free_world << <1, 1 >> > (d_list, d_world);
    checkCudaErrors(cudaFree(d_list));
    checkCudaErrors(cudaFree(d_world));
    checkCudaErrors(cudaFree(fb));
}