#pragma once

#include "bvh_node.h"
#include "surface.h"

namespace plu {

	namespace surfaces {

		struct bvh_tree : public surface {
			bvh_node root;

			bvh_tree(vector<shared_ptr<surface>>& s) {
				//need to sort surface list somehow
				//set left node == bvh_tree of left list, right == bvh_tree of right list
				//recursive base case: list size == 1, or 2
				//set parent of both nodes == root

				std::sort(s.begin(), s.end(), bvh_tree.lt);

				if(s.size() == 1) {
					root = bvh_node(null, s[0]);
				} else if (s.size() == 2) {
					root = bvh_node(null, bvh_node(root, s[0]), bvh_node(root, s[1]), aabb(s[0].bounds(), s[1].bounds()));
				} else {
					int mid = s.size()/2;
					aabb bounds = s[0].bounds;
					for(std::vector<shared_ptr<surface>>::iterator i = s.begin()+1; i != v.end(); ++i) {
						bounds.add_aabb(s[i].bounds());
					}

					vector<shared_ptr<surface>> lv(s.begin(), s.begin()+mid);
					vector<shared_ptr<surface>> rv(s.begin()+mid, s.end());
					root = bvh_node(null, bvh_tree(lv).root, bvh_tree(rv).root, bounds);
				}

			}

			inline aabb bounds() const override {
				return root.bounds;
			}

			bool hit(const ray& r, hit_record * hr) const override;

			void sort(vector<shared_ptr<surface>>);

			bool lt(vec3 c1, vec3 c2);
		};

	}
}
