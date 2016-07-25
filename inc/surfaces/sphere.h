#pragma once
#include "surface.h"

namespace plu {
	namespace surfaces {
		struct sphere : public surface {
			vec3 center; float radius;

			sphere(vec3 c, float r) : center(c), radius(r) {}

			inline aabb bounds() override {
				return aabb(center-radius, center+radius);
			}

			bool hit(const ray& r, hit_record* hr) const override;
		};
	}
}
