#include "surfaces\box.h"

namespace plu {
	namespace surfaces {
		bool box::hit(const ray& r, hit_record* hr) const {
			return this->Bounds().hit(r);
		}
	}
}