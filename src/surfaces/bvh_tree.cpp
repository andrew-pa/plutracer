#include "surfaces\bvh_tree.h"

namespace plu {
	namespace surfaces {
		bool bvh_tree::hit(const ray&r, hit_record * hr) const{
			return root.hit(r, hr);
		}
	}
}