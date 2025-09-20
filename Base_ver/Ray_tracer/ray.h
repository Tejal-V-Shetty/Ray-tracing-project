#ifndef RAYH
#define RAYH
#include "vector3.h"

// A ray is represented by a function p(t) = A+t*B
//	where A: Ray origin and B: Ray direction
class ray
{
	public:
		ray() {}
		ray(const vector3& a, const vector3& b) 
		{ 
			A = a; 
			B = b; 
		}
		vector3 origin() const { return A; }
		vector3 direction() const { return B; }
		vector3 point_at_parameter(float t) const { return A + t * B; }

		vector3 A;
		vector3 B;
};

#endif