#pragma once
#include "surface.h"

namespace plu {
	namespace surfaces {
		struct box : public surface {
			aabb box_bounds;

			box(vec3 c, vec3 e) : box_bounds(c-e,c+e) {}

			inline aabb bounds() const override {
				return box_bounds;
			}

			bool hit(const ray& r, hit_record* hr) const override;

			vec3 box::get_normal(vec3 p) const;
		};
	}
}