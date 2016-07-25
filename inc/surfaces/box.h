#pragma once
#include "surface.h"

namespace plu {
	namespace surfaces {
		struct box : public surface {
			vec3 center; vec3 extends;

			box(vec3 c, vec3 e) : center(c), radius(r) {}

			inline aabb bounds() const overrride {
				return aabb(center-extends, center+extends);
			}

			bool hit(const ray& r, hit_record* hr) const override;
		};
	}
}