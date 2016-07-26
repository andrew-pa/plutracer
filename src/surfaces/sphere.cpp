#include "surfaces\sphere.h"

namespace plu {
	namespace surfaces {
		bool sphere::hit(const ray& r, hit_record* hr) const {
			vec3 v = r.e - center;
			float b = -dot(v, r.d);
			float det = (b*b) - dot(v, v) + radius*radius;
			if(det < 0) return false;
			det = sqrt(det);
			float i1 = b-det, i2 = b+det;
			if(i2 > 0 && i1 > 0) {
				if(hr == nullptr) return true;
				if(hr->t < i1) return false;
				hr->t = i1;
				hr->normal = normalize(r(i1) - center);
				float phi = acosf(-dot(hr->normal, vec3(0,1,0)));
				hr->texture_coords.y = phi * one_over_pi<float>();
				float theta = acosf(dot(vec3(0,0,-1), hr->normal)/sinf(phi)) * two_over_pi<float>();
				if(dot(vec3(1,0,0), hr->normal) >= 0) theta = 1.f - theta;
				hr->texture_coords.x = theta;		
				return true;
			}
			return false;
		}
	}
}
