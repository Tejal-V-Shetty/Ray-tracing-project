#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

#ifndef CAMERAH
#define CAMERAH

#include "ray.h"
#include <math.h>

vector3 random_in_unit_disk() {
	vector3 p;
	do {
		p = 2.0 * vector3((double)rand() / RAND_MAX, (double)rand() / RAND_MAX, 0) - vector3(1, 1, 0);
	} while (dot(p, p) >= 1.0);
	return p;
}

class camera {
public:
	camera(vector3 lookfrom, vector3 lookat, vector3 vup, float vfov, float aspect, float aperture, float focus_dist) { // Vertical field of view is top to bottom in degrees
		lens_radius = aperture / 2;
		float theta = vfov * M_PI / 180;
		float half_height = tan(theta / 2);
		float half_width = aspect * half_height;
		origin = lookfrom;
		w = unit_vector(lookfrom - lookat);
		u = unit_vector(cross(vup, w));
		v = cross(w, u);
		lower_left_corner = origin - half_width * focus_dist * u - half_height * focus_dist * v - focus_dist * w;
		horizontal = 2 * half_width * focus_dist * u;
		vertical = 2 * half_height * focus_dist * v;
	}
	ray get_ray(float s, float t) {
		vector3 rd = lens_radius * random_in_unit_disk();
		vector3 offset = u * rd.x() + v * rd.y();
		return ray(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin - offset);
	}

	vector3 origin;
	vector3 lower_left_corner;
	vector3 horizontal;
	vector3 vertical;
	vector3 u, v, w;
	float lens_radius;
};
#endif