#include "cmmn.h"
#include "surface.h"

namespace plu {
	namespace surfaces {
		struct triangle : public surface {
			vec3* vertices[3];
			vec3* normals[3];
			vec2* texture_coords[3];

			triangle(vec3* va, vec3* vb, vec3* vc, vec3* na, vec3* nb, vec3* nc, vec2* ta, vec2* tb, vec2* tc, shared_ptr<material> mat)
				: surface(mat)
			{
				vertices[0] = va; vertices[1] = vb; vertices[2] = vc;
				normals[0] = na; normals[1] = nb; normals[2] = nc;
				texture_coords[0] = ta; texture_coords[1] = tb; texture_coords[2] = tc;
			}
			
			bool hit(const ray& r, hit_record* hr) const override;
			inline aabb bounds() const override {
				auto b = aabb();
				for (int i = 0; i < 3; ++i) b.add_point(*vertices[i]);
				return b;
			}
			float area() const override {
				float a = distance(*vertices[0], *vertices[1]);
				float b = distance(*vertices[1], *vertices[2]);
				float c = distance(*vertices[0], *vertices[2]);
				float p = (a + b + c)*0.5f;
				return sqrt(p*(p - a)*(p - b)*(p - c));
			}
			vec3 sample(vec2 u, vec3* n) const override;
		};
	}
}
