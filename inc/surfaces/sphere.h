#pragma once
#include "surface.h"

namespace plu {
	namespace surfaces {
		struct sphere : public surface {
			vec3 center; float radius;

			sphere(vec3 c, float r, shared_ptr<material> m) : center(c), radius(r), surface(m) {}

			inline aabb bounds() const override {
				return aabb(center-radius, center+radius);
			}

			bool hit(const ray& r, hit_record* hr) const override;

			float area() const override { return (4.f/3.f)*pi<float>()*radius*radius*radius; }
			vec3 sample(vec2 u, vec3* n) const override {
				vec3 np = rnd::uniform_sphere_sample(u);
				*n = np;
				return center+np*radius;
			}
		};
	}
}
