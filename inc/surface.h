#pragma once
#include "cmmn.h"
#include "material.h"

namespace plu {
	struct surface {
		// does ray r intersect this surface?
		// should return false if the ray does not intersect the surface
		// 	OR if the point of intersection is farther along the ray than the current point of intersection reprsented by hr
		// if this function returns true, it is assumed that unless hr is nullptr that it has set hr (potentially with its set function)
		// to represent the point of intersection found
		virtual bool hit(const ray& r, hit_record* hr) const = 0;

		// return an AABB that surrounds (hopefully fairly tightly) the surface
		virtual aabb bounds() const = 0;

		virtual float area() const { throw runtime_error("unimplmented surface::area"); }

		// return a random point on the surface
		// TODO: could be rewritten to return a tuple
		virtual vec3 sample(vec2 u, vec3* n) const { throw runtime_error("unimplemented surface::sample"); }
		virtual vec3 sample(vec3 P, vec2 u, vec3* n) const { return sample(u,n); }

		// returns the probablity that the point given will be sampled
		virtual float pdf(vec3 p) const { return 1.f / area(); }
		virtual float pdf(vec3 p, vec3 wi) const {
			hit_record hr;
			if(!hit(ray(p,wi),&hr)) return 0.f;
			else {
				vec3 D = p + wi*hr.t;
				return dot(D,D) / (abs(dot(hr.norm,-wi)) * area());
			}
		}	

		// the material for this surface
		shared_ptr<material> mat;

	protected:
		// provide a constructor for implmentations to set the material upon construction
		surface(shared_ptr<material> m = nullptr) : mat(m) {}
	};

}
