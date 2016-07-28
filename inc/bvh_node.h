#pragma once

#include <list>
#include "cmmn.h"


 namespace plu {

 	struct bvh_node : public aabb {
 	public:
 		bvh_node * parent; bvh_node * left_child; bvh_node * right_child;

 		bvh_node()
 		: aabb() {
 			parent = nullptr;
 			left_child = nullptr;
 			right_child = nullptr;
 		}

 		bvh_node(bvh_node * p, bvh_node * lc, bvh_node * rc, vec3 mi, vec3 mx)
 		: aabb(mi, mx) {
 			parent = p;
 			left_child = lc;
 			right_child = rc;
 		}

 		bool is_leaf() {
 			return (left_child == nullptr && right_child == nullptr);
 		}
 	}
 }