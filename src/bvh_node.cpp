#pragma once
#include "bvh_node.h"

namespace plu {

	//checks surface collision if object is leaf else checks intersection with aabb
	bool bvh_node::hit(const ray& r, hit_record* hr) {
		if(object != nullptr) {
			return object->hit(const ray& r, hit_record * hr);
		}

		//create hr for each side so they can be compared
		hit_record * lhr = new hit_record();
		hit_record * rhr = new hit_record();

		bool lhit = left_child->hit(r, lhr);
		bool rhit = right_child->hit(r, rhr);

		//no hit
		if(lhit == false && rhit == false) {
			return false;
		}

		//right hit only
		if(lhit == false) {
			if(hr == nullptr) {
				return true;
			}
			hr->set(rhr->surf, rhr->t, vec3(), rhr->normal, rhr->texture_coords);
			return true;
		}

		//right hit only
		if(rhit == false) {
			if(hr == nullptr) {
				return true;
			}
			hr->set(lhr->surf, lhr->t, vec3(), lhr->normal, lhr->texture_coords);
			return true;
		}

		//both hit
		if(hr == nullptr) {
			return true;
		}
		//set hr to closest hit
		if(lhr->t < rhr->t) {
			hr->set(lhr->surf, lhr->t, vec3(), lhr->normal, lhr->texture_coords);
			return true;
		}

		hr->set(rhr->surf, rhr->t, vec3(), rhr->normal, rhr->texture_coords);
		return true;
	}
}