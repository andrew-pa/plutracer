#pragma once

#include "surface.h"

namespace plu {

	namespace surfaces {

		class bvh_tree : public surface {

			class bvh_node {
			public:
				bvh_node* parent;
				unique_ptr<bvh_node> left_child;
				unique_ptr<bvh_node> right_child;

				aabb bounds;

				shared_ptr<surface> object;

				bvh_node()
					: parent(nullptr), left_child(nullptr), right_child(nullptr), object(nullptr) {
					bounds = aabb();
				}
				bvh_node(bvh_node* p, vector<shared_ptr<surface>>& s, int axis = 0);

				bool hit(const ray& r, hit_record* hr) const;

				bvh_node(bvh_node* p, shared_ptr<surface> o)
					: parent(p), left_child(nullptr), right_child(nullptr), object(o) {
					bounds = object->bounds();
				}
			};

			unique_ptr<bvh_node> root;
		public:
			bvh_tree(vector<shared_ptr<surface>>& s) {
				root = make_unique<bvh_node>(nullptr, s);
			}

			inline aabb bounds() const override {
				return root->bounds;
			}

			bool hit(const ray& r, hit_record* hr) const override {
				return root->hit(r, hr);
			}

		};

	}
}
