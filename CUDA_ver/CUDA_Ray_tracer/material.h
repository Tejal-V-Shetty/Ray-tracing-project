#ifndef MATERIALH
#define MATERIALH

#include "vector3.h"
#include "ray.h"
#include "hitablelist.h"
#include "sphere.h"
#include "camera.h"

__device__ vector3 reflect(const vector3& v, const vector3& n){
	return v - 2.0f * dot(v, n) * n;
}

__device__ bool refract(const vector3& v, const vector3& n, float ni_over_nt, vector3& refracted) {
	vector3 uv = unit_vector(v);
	float dt = dot(uv, n);
	float discriminant = 1.0f - ni_over_nt * ni_over_nt * (1.0f - dt * dt);
	if (discriminant > 0.0f) {
		refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
		return true;
	}
	else
		return false;
}

__device__ float schlick(float cosine, float ref_idx) {
	float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
	r0 = r0 * r0;
	return r0 + (1.0f - r0) * pow((1.0f - cosine), 5.0f);
}

//For diffuse surfaces, a random point in the unit sphere drawn at the tangent to the contact point is required
__device__ vector3 random_in_unit_sphere(curandState* rand_state) {
	vector3 p;
	do {
		p = 2.0f * vector3(curand_uniform(rand_state), curand_uniform(rand_state), curand_uniform(rand_state)) - vector3(1, 1, 1);
	} while (p.squared_length() >= 1.0f);
	return p;
}

class material {
public:
	__device__ virtual bool scatter(const ray& r_in, const hit_record& rec, vector3& attenuation, ray& scattered, curandState* rand_state) const = 0;
};

class lambertian : public material {
public:
	__device__ lambertian(const vector3& a) : albedo(a) {}
	__device__ virtual bool scatter(const ray& r_in, const hit_record& rec, vector3& attenuation, ray& scattered, curandState* rand_state) const {
		vector3 target = rec.p + rec.normal + random_in_unit_sphere(rand_state);
		scattered = ray(rec.p, target - rec.p);
		attenuation = albedo;
		return true;
	}

	vector3 albedo;
};

class metal : public material {
public:
	__device__ metal(const vector3& a, float f) :albedo(a) {
		if (f < 1.0f) 
			fuzz = f;
		else 
			fuzz = 1.0f;
	}
	__device__ virtual bool scatter(const ray& r_in, const hit_record& rec, vector3& attenuation, ray& scattered, curandState* rand_state) const {
		vector3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected+fuzz*random_in_unit_sphere(rand_state));
		attenuation = albedo;
		return (dot(scattered.direction(),rec.normal)>0.0f);
	}
	vector3 albedo;
	float fuzz;
};

class dielectric : public material {
public:
	__device__ dielectric(float ri) : ref_idx(ri) {}
	__device__ virtual bool scatter(const ray& r_in, const hit_record& rec, vector3& attenuation, ray& scattered, curandState* rand_state) const {
		vector3 outward_normal;
		vector3 reflected = reflect(r_in.direction(), rec.normal);
		float ni_over_nt;
		attenuation = vector3(1.0, 1.0, 1.0);
		vector3 refracted;
		float reflect_prob;
		float cosine;
		if (dot(r_in.direction(), rec.normal) > 0) {
			outward_normal = -rec.normal;
			ni_over_nt = ref_idx;
			cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}
		else {
			outward_normal = rec.normal;
			ni_over_nt = 1.0 / ref_idx;
			cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}
		if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted)) {
			reflect_prob = schlick(cosine, ref_idx);
		}
		else {
			reflect_prob = 1.0;
		}
		if ((double)rand() / RAND_MAX < reflect_prob) {
			scattered = ray(rec.p, reflected);
		}
		else {
			scattered = ray(rec.p, refracted);
		}
		return true;
	}

	float ref_idx;
};
#endif