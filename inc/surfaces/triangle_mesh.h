#pragma once
#include "surface.h"

namespace plu {
	namespace surfaces {
		class triangle_mesh : public surface {

			struct node {
				triangle_mesh* parent;
				union {
					struct {
						unique_ptr<bvh_node> left, right;
					};
					size_t index_start, triangle_count;
				};
				aabb bounds;

				bool hit(const ray& r, hit_record* hr) const;
			};

			bool intersect_triangle(const ray& r, hit_record* hr, size_t i) const;
public:
			vector<vec3> vertices;
			vector<uint32_t> indices;

		};
	}
}
