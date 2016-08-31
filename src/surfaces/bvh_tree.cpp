#pragma once
#include "surfaces/bvh_tree.h"

namespace plu {
	namespace surfaces {

		bvh_tree::bvh_node::bvh_node(bvh_node* p, vector<shared_ptr<surface>>& s, int axis) : parent(p) {
			//need to sort surface list somehow
			//set left node == bvh_tree of left list, right == bvh_tree of right list
			//recursive base case: list size == 1, or 2
			//set parent of both nodes == root
			
			std::sort(s.begin(), s.end(), [axis](shared_ptr<surface> s1, shared_ptr<surface> s2) {
				vec3 c1 = s1->bounds().center();
				vec3 c2 = s2->bounds().center();
				return c1[axis] < c2[axis];
			});

			if (s.size() == 1) {
				object = s[0];
				bounds = object->bounds();
			}
			else if (s.size() == 2) {
				left_child = make_unique<bvh_node>(this, s[0]);
				right_child = make_unique<bvh_node>(this, s[1]);
				bounds = aabb(left_child->bounds, right_child->bounds);
			}
			else {
				int mid = s.size() / 2;
				vector<shared_ptr<surface>> lv(s.begin(), s.begin() + mid);
				vector<shared_ptr<surface>> rv(s.begin() + mid, s.end());
				left_child = make_unique<bvh_node>(this, lv, (axis + 1) % 3);
				right_child = make_unique<bvh_node>(this, rv, (axis + 1) % 3);
				bounds = aabb(left_child->bounds, right_child->bounds);
			}
		}

		//checks surface collision if object is leaf else checks intersection with aabb
		bool bvh_tree::bvh_node::hit(const ray& r, hit_record* hr) const {
			if (object != nullptr) {
				return object->hit(r, hr);
			}

			//check if ray intersects node aabb
			if(!bounds.hit(r))
				return false;

			//create hr for each side so they can be compared
			hit_record lhr, rhr;

			bool lhit = left_child->hit(r, &lhr);
			bool rhit = right_child->hit(r, &rhr);

			//no hit
			if (lhit == false && rhit == false)
				return false;
			
			//right hit only
			if (lhit == false) {
				if (hr == nullptr)
					return true;
				*hr = rhr;
				return true;
			}

			//right hit only
			if (rhit == false) {
				if (hr == nullptr)
					return true;
				*hr = lhr;
				return true;
			}

			//both hit
			if (hr == nullptr)
				return true;
			//set hr to closest hit
			if (lhr.t < rhr.t) {
				*hr = lhr;
				return true;
			}

			*hr = rhr;
			return true;
		}
	}
}
