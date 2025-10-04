#ifndef MATERIALH
#define MATERIALH

#include <vector3.h>
#include "ray.h"
#include "hitablelist.h"
#include "sphere.h"
#include "camera.h"

vector3 reflect(const vector3& v, const vector3& n){
	return v - 2 * dot(v, n) * n;
}

//For diffuse surfaces, a random point in the unit sphere drawn at the tangent to the contact point is required
vector3 random_in_unit_sphere() {
	vector3 p;
	do {
		p = 2.0 * vector3((double)rand() / RAND_MAX, (double)rand() / RAND_MAX, (double)rand() / RAND_MAX) - vector3(1, 1, 1);
	} while (p.squared_length() >= 1.0);
	return p;
}

class material {
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, vector3& attenuation, ray& scattered) const = 0;
};

class lambertian : public material {
public:
	lambertian(const vector3& a) : albedo(a) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vector3& attenuation, ray& scattered) const {
		vector3 target = rec.p + rec.normal + random_in_unit_sphere();
		scattered = ray(rec.p, target - rec.p);
		attenuation = albedo;
		return true;
	}

	vector3 albedo;
};

class metal : public material {
public:
	metal(const vector3& a, float f) :albedo(a) {
		if (f < 1) 
			fuzz = f;
		else 
			fuzz = 1;
	}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vector3& attenuation, ray& scattered) const {
		vector3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected+fuzz*random_in_unit_sphere());
		attenuation = albedo;
		return (dot(scattered.direction(),rec.normal)>0);
	}
	vector3 albedo;
	float fuzz;
};
#endif