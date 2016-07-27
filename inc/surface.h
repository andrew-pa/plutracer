#pragma once
#include "cmmn.h"

#include "props.h"

namespace plu {

	// this is preliminary and should probably get it's own header soon
	struct material {
		props::color diffuse;
		material(props::color df) : diffuse(df) {}
	};
	
	struct surface {
		// does ray r intersect this surface?
		// should return false if the ray does not intersect the surface
		// 	OR if the point of intersection is farther along the ray than the current point of intersection reprsented by hr
		// if this function returns true, it is assumed that unless hr is nullptr that it has set hr (potentially with its set function)
		// to represent the point of intersection found
		virtual bool hit(const ray& r, hit_record* hr) const = 0;

		// return an AABB that surrounds (hopefully fairly tightly) the surface
		virtual aabb bounds() const = 0;

		shared_ptr<material> mat;

	protected:
		surface(shared_ptr<material> m = nullptr) : mat(m) {}
	};

}
