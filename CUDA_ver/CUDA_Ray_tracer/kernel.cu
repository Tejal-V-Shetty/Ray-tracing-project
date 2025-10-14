
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <curand_kernel.h>
#include <stdio.h>
#include <iostream>
#include <limits>
#include "vector3.h"
#include "ray.h"
#include "hitablelist.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

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

__global__ void rand_init(curandState* rand_state) 
{
    if (threadIdx.x == 0 && blockIdx.x == 0) {
        curand_init(2001, 0, 0, rand_state);//Same seed for all threads
    }
}

__global__ void create_world(hitable **d_list, hitable **d_world, camera ** d_cam, int nx, int ny, curandState* rand_state)
{
    if (threadIdx.x == 0 && blockIdx.x == 0)    //Only initialize them once
    {
        d_list[0] = new sphere(vector3(0, -1000, 0), 1000, new lambertian(vector3(0.5, 0.5, 0.5)));
        int i = 1;
        for (int a = -11; a < 11; a++) {
            for (int b = -11; b < 11; b++) {
                float choose_mat = curand_uniform(rand_state);
                vector3 center(a + 0.9f * curand_uniform(rand_state), 0.2f, b + 0.9f * curand_uniform(rand_state));
                    if (choose_mat < 0.8f) {
                        d_list[i++] = new sphere(center, 0.2, new lambertian(vector3(curand_uniform(rand_state) * curand_uniform(rand_state), curand_uniform(rand_state) * curand_uniform(rand_state), curand_uniform(rand_state) * curand_uniform(rand_state))));
                    }
                    else if (choose_mat < 0.95f) {
                        d_list[i++] = new sphere(center, 0.2, new metal(vector3(0.5 * (1 + curand_uniform(rand_state)), 0.5 * (1 + curand_uniform(rand_state)), 0.5 * (1 + curand_uniform(rand_state))), 0.5 * curand_uniform(rand_state)));

                    }
                    else {
                        d_list[i++] = new sphere(center, 0.2, new dielectric(1.5));
                    }
            }
        }
        d_list[i++] = new sphere(vector3(0, 1, 0), 1.0, new dielectric(1.5));
        d_list[i++] = new sphere(vector3(-4, 1, 0), 1.0, new lambertian(vector3(0.4, 0.2, 0.1)));
        d_list[i++] = new sphere(vector3(4, 1, 0), 1.0, new metal(vector3(0.7, 0.6, 0.5), 0.0));
        *d_world = new hitable_list(d_list, 488);
        vector3 lookfrom(13, 2, 3);
        vector3 lookat(0, 0, 0);
        float dist_to_focus = 10.0f;//(lookfrom - lookat).length();
        float aperture = 0.1f;
        *d_cam = new camera(lookfrom, lookat, vector3(0, 1, 0), 30, float(nx) / float(ny), aperture, dist_to_focus);
    }
}

__global__ void free_world(hitable** d_list, hitable** d_world, camera **d_cam)
{
    for (int i = 0; i < 488; i++){
        delete((sphere*)d_list[i])->mat_ptr;
        delete d_list[i];
    }
    delete* d_world;
    delete* d_cam;
}

__global__ void render_init(int max_x, int max_y, curandState* rand_state)
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;
    if ((i >= max_x) || (j >= max_y))
        return;
    int pixel_index = j * max_x + i;
    curand_init(2001, pixel_index, 0, &rand_state[pixel_index]);//Same seed for all threads
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

__device__ vector3 color(const ray& r, hitable **world, curandState *rand_state)
{
    ray cur_ray = r;
    vector3 cur_attenuation = vector3(1.0, 1.0, 1.0);
    for (int i = 0; i < 50; i++) {
        hit_record rec;
        if ((*world)->hit(cur_ray, 0.001f, FLT_MAX, rec)) {
            ray scattered;
            vector3 attenuation;
            if (rec.mat_ptr->scatter(cur_ray, rec, attenuation, scattered, rand_state))
            {
                cur_attenuation *= attenuation;
                cur_ray = scattered;
            }
            else {
                return vector3(0.0, 0.0, 0.0);
            }
        }
        else {
            vector3 unit_dir = unit_vector(cur_ray.direction());
            float t = 0.5f * (unit_dir.y() + 1.0f);
            return cur_attenuation*((1.0f - t) * vector3(1.0, 1.0, 1.0) + t * vector3(0.5, 0.7, 1.0)); //LERP between white and blue
        }
    }
    return vector3(0.0, 0.0, 0.0);
}

__global__ void render(vector3* fb, int max_x, int max_y, int ns, camera **cam, hitable **world, curandState *rand_state)
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;
    if ((i >= max_x) || (j >= max_y))
        return;
    int pixel_index = j * max_x + i;
    curandState local_rand_state = rand_state[pixel_index];
    vector3 col(0, 0, 0);
    for (int s = 0; s < ns; s++)
    {
        float u = float(i + curand_uniform(&local_rand_state)) / float(max_x);
        float v = float(j + curand_uniform(&local_rand_state)) / float(max_y);
        ray r = (*cam)->get_ray(u, v, &local_rand_state);
        col += color(r, world, &local_rand_state);
    }
    rand_state[pixel_index] = local_rand_state;
    col /= float(ns);
    col[0] = sqrt(col[0]);
    col[1] = sqrt(col[1]);
    col[2] = sqrt(col[2]);
    fb[pixel_index] = col;
}

int main()
{
    int nx = 1200;
    int ny = 600;
    int ns = 100;
    int num_pixels = nx * ny;
    size_t fb_size = num_pixels * sizeof(vector3);

    curandState* d_rand_state;
    checkCudaErrors(cudaMalloc((void **)&d_rand_state, num_pixels*sizeof(curandState)));
    curandState* d_rand_state_2;
    checkCudaErrors(cudaMalloc((void**)&d_rand_state_2, 1 * sizeof(curandState)));

    rand_init << <1, 1 >> > (d_rand_state_2);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    //Frame buffer allocation
    vector3* fb;
    checkCudaErrors(cudaMallocManaged((void**) & fb, fb_size));

    vector3 lower_left_corner(-2.0, -1.0, -1.0);
    vector3 horizontal(4.0, 0.0, 0.0);
    vector3 vertical(0.0, 2.0, 0.0);
    vector3 origin(0.0, 0.0, 0.0);

    int num_hitables = 488;//(22 * 22) + 1 + 3;
    hitable** d_list;   //The objects in the world
    checkCudaErrors(cudaMalloc((void**)&d_list, num_hitables * sizeof(hitable*)));
    hitable** d_world;
    checkCudaErrors(cudaMalloc((void**)&d_world, sizeof(hitable *)));
    camera** d_cam;
    checkCudaErrors(cudaMalloc((void**)&d_cam, sizeof(camera*)));
    create_world << <1, 1 >> > (d_list, d_world, d_cam, nx, ny, d_rand_state_2);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    int tx = 8, ty = 8; //Threads
    dim3 blocks(nx / tx + 1, ny / ty + 1);
    dim3 threads(tx, ty);
    render_init << <blocks, threads >> > (nx, ny, d_rand_state);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());
    render<<<blocks, threads >>> (fb, nx, ny, ns, d_cam, d_world, d_rand_state);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    //Output the image
    freopen("out_Ch12.ppm", "w", stdout);
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

    checkCudaErrors(cudaDeviceSynchronize());
    free_world << <1, 1 >> > (d_list, d_world, d_cam);

    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaFree(d_list));
    checkCudaErrors(cudaFree(d_world));
    checkCudaErrors(cudaFree(d_rand_state));
    checkCudaErrors(cudaFree(d_cam));
    checkCudaErrors(cudaFree(fb));
}