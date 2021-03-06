#pragma once
#include "surface.h"

namespace plu {
	namespace surfaces {
		struct box : public surface {
			aabb box_bounds;

			box(vec3 c, vec3 e, shared_ptr<material> m) : box_bounds(c-e,c+e), surface(m) {}

			inline aabb bounds() const override {
				return box_bounds;
			}

			bool hit(const ray& r, hit_record* hr) const override;

			vec3 get_normal(vec3 p) const;

			inline float area() const override {
				return box_bounds.surface_area();
			}
			
			vec3 sample(vec2 u, vec3* n) const override {
				vec3 U = vec3(u.x, rnd::randf(), u.y);
				int mi = rnd::randi(0, 2);
				U[mi] = U[mi] > 0.5f ? 1.f : 0.f;
				vec3 p = box_bounds._min + U*box_bounds.extents();
				*n = vec3(0.f);
				(*n)[mi] = U[mi] > 0.5f ? 1.f : -1.f;
				/*if (*n != get_normal(p)) {
					cout << *n << "  " << get_normal(p) << endl;
					throw *n;
				}/*/
				return p;
				/*int face = (int)floor(u.x*6.f);
				u.x = fract(u.x*6.f);
				vec3 ex = box_bounds.extents();
				vec2 t = vec2(ex[(face+1) % 3], ex[(face+2)%3]);
				t *= u;
				vec3 v;
				v[(face + 1) % 3] = t.x;
				v[(face + 2) % 3] = t.y;
				v[(face % 3)] = ex[face % 3] * (face > 3 ? -1.f : 1.f);
				*n = vec3(0.f);
				(*n)[face % 3] = face < 3 ? 1.f : -1.f;
				return v;*/
			}
		};
	}
}
