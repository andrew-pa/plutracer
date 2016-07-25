#pragma once
#include "surface.h"

namespace plu {
	namespace surfaces {
		struct box : public surface {
			aabb bounds;

			box(vec3 c, vec3 e) : {}

			bool hit(const ray& r, hit_record* hr) const override;

			vec3 box::get_normal(vec3 p);
		};
	}
}