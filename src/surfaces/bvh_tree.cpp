#include "surfaces\bvh_tree.h"

namespace plu {
	namespace surfaces {
		bool bvh_tree::hit(const ray&r, hit_record * hr) const {
			return root.hit(r, hr);
		}

		//lambda for 'comp' argument in std::sort
		bool bvh_tree::lt(surface s1, surface s2) {
			vec3 c1 = s1.bounds().center();
			vec3 c2 = s2.bounds().center();

			if(c1.x < c2.x) {
				return true;
			} else if (c1.x > c2.x) {
				return false;
			} else {
				if(c1.y < c2.y) {
					return true;
				} else if (c1.y > c2.y) {
					return false;
				} else {
					return c1.z < c2.z;
				}
			}
		}
	}
}