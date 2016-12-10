#include "surfaces/triangle_mesh.h"

namespace plu {
	namespace surfaces {
		bool triangle_mesh::intersect_triangle(const ray& r, hit_record* hr, size_t i) const {

		}	
		bool triangle_mesh::node::hit(const ray& r, hit_record* hr) const {
			if(parent == nullptr) {
				for(size_t i = index_start; i < triangle_count; ++i) {
					if(parent->intersect_triangle(r,hr,i)) return true;
				}	
				return false;
			}

			if(!bounds.hit(r)) return false;
			
			hit_record lhr, rhr;

			bool lhit = left->hit(r, &lhr), rhit = right->hit(r, &rhr);
			if(!lhit && !rhit) return false;

			if(!lhit) {
				if(hr != nullptr) *hr = rhr;
				return true;
			}			

			if(!rhit) {
				if(hr != nullptr) *hr = lhr;
				return true;
			}

			if(hr != nullptr) 
				*hr = lhr.t < rhr.t ? lhr : rhr;

			return true;
		}
	}
}
