#include "surfaces\box.h"

namespace plu {
	namespace surfaces {

		box::box(vec3 c, vec3 e) {
			bounds = aabb(c-e, c+e);
		}

		bool box::hit(const ray& r, hit_record* hr) const {

			vec3 rrd = 1.f / r.d;

			vec3 t1 = (bounds.min - r.e) * rrd;
			vec3 t2 = (bounds.max - r.e) * rrd;

			vec3 m12 = glm::min(t1, t2);
			vec3 x12 = glm::max(t1, t2);

			float tmin = m12.x;
			tmin = glm::max(tmin, m12.y);
			tmin = glm::max(tmin, m12.z);

			float tmax = x12.x;
			tmax = glm::min(tmax, x12.y);
			tmax = glm::min(tmax, x12.z);

			if(tmax < tmin || tmin < 0) return false
			if(hr == nullptr) return true;
			if(tmin > hr->t) return false;
			vec3 p = r(tmin);
			normal = get_normal(p);
			tc = vec2(p.x, p.z);
			hr.set(tmin, p, normal, tc);
			return true;
		}

		vec3 box::get_normal(vec3 p) {
			static const vec3 axises[] =
			{
				vec3(1,0,0),
				vec3(0,1,0),
				vec3(0,0,1),
			};
			vec3 n = vec3(0);
			float m = FLT_MAX;
			float dist;
			vec3 np = p - center;
			for (int i = 0; i < 3; ++i)
			{
				dist = fabsf(extents[i] - fabsf(np[i]));
				if(dist < m)
				{
					m = dist;
					n = sign(np[i])*axises[i];
				}
			}
			return n;
		}
	}
}