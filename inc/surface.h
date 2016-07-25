#pragma once
#include "cmmn.h"

namespace plu {

	struct hit_record {
		float t;
		vec3 position, normal;
		vec2 texture_coords;

		hit_record() : t(100000.f) {}

		// convience function that sets the hit_record's values in place
		inline void set(float _t, vec3 p, vec3 n, vec2 tc) {
			t = _t; position = p; normal = n; texture_coords = tc;
		}
	};
	
	struct surface {
		// does ray r intersect this surface?
		// should return false if the ray does not intersect the surface
		// 	OR if the point of intersection is farther along the ray than the current point of intersection reprsented by hr
		// if this function returns true, it is assumed that unless hr is nullptr that it has set hr (potentially with its set function)
		// to represent the point of intersection found
		virtual bool hit(const ray& r, hit_record* hr) const = 0;
	};

}
