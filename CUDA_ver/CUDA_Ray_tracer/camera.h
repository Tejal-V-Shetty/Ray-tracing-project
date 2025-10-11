#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef CAMERAH
#define CAMERAH

#include "ray.h"
#include <math.h>


class camera {
public:
	__device__ __host__ camera(vector3 lookfrom, vector3 lookat, vector3 vup, float vfov, float aspect) { // Vertical field of view is top to bottom in degrees
		vector3 u, v, w;
		float theta = vfov * M_PI / 180;
		float half_height = tan(theta / 2);
		float half_width = aspect * half_height;
		origin = lookfrom;
		w = unit_vector(lookfrom - lookat);
		u = unit_vector(cross(vup, w));
		v = cross(w, u);
		lower_left_corner = origin - half_width * u - half_height * v - w;
		horizontal = 2 * half_width * u;
		vertical = 2 * half_height * v;
	}
	__device__ ray get_ray(float u, float v) {
		return ray(origin, lower_left_corner + u * horizontal + v * vertical - origin);
	}

	vector3 origin;
	vector3 lower_left_corner;
	vector3 horizontal;
	vector3 vertical;
};
#endif