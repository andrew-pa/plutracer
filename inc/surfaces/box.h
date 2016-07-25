#pragma once
#include "surface.h"

namespace plu {
	namespace surfaces {
		struct box : public surface {
			vec3 center; vec3 extents; aabb bounds;

			box(vec3 c, vec3 e) : center(c), radius(r) {}

			inline aabb bounds() const override {
				return aabb(center-extents, center+extents);
			}

			bool hit(const ray& r, hit_record* hr) const override;

			vec3 box::get_normal(vec3 p);
		};
	}
}