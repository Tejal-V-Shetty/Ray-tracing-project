#ifndef SPHEREH
#define SPHEREH

#include "hitable.h"

class sphere : public hitable {
public:
    __device__ sphere() {}
    __device__ sphere(vector3 cen, float r) : center(cen), radius(r) {};
    __device__ virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
	vector3 center;
	float radius;
};

//Calculates a sphere hit based on an expanded version of the formula for a sphere.
//Formula: dot(p(t)-c, p(t)-c) = R*R [Derived from x*x + y*y + z*z = R*R]
__device__ bool sphere::hit(const ray& r, float tmin, float tmax, hit_record& rec) const {
    vector3 oc = r.origin() - center;
    float a = dot(r.direction(), r.direction());
    float b = 2.0 * dot(oc, r.direction());
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant > 0) {
        float temp = (-b - sqrt(discriminant)) / (2.0 * a);
        if (temp<tmax && temp>tmin){
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            return true;
        }
        temp = (-b + sqrt(discriminant)) / (2.0 * a);
        if (temp<tmax && temp>tmin) {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            return true;
        }
    }
    return false;
}

#endif