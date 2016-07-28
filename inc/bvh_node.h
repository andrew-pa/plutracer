#pragma once

#include "cmmn.h"
#include "surfaces\surfaces.h"


 namespace plu {

 	struct bvh_node {
 	public:
 		bvh_node * parent;
 		bvh_node * left_child;
 		bvh_node * right_child;

 		aabb bounds;

 		surface * object;

 		bvh_node() 
 		: parent(nullptr), left_child(nullptr), right_child(nullptr), object(nullptr) {
 			bounds = aabb();
 		}

 		bvh_node(bvh_node * p, bvh_node * lc, bvh_node * rc, vec3 mi, vec3 mx)
 		: parent(p), left_child(lc), right_child(rc), object(nullptr) {
 			bounds = aabb(mi, mx);
 		}

 		bvh_node(bvh_node * p, surface * o) 
 		: parent(p), left_child(nullptr), right_child(nullptr), object(o) {
 			bounds = object->bounds();
 		}

 		bool hit(const ray& r, hit_record * hr);
 	}
 }