#include bvh_node.h

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

		//set hr to closest hit
		if(lhr->t < rhr->t) {
			hr->set(lhr->surf, lhr->t, vec3(), lhr->normal, lhr->tc);
			return lhit;
		}

		hr->set(rhr->surf, rhr->t, vec3(), rhr->normal, rhr->tc);
		return rhit;
	}
}