#include "surfaces\box.h"

namespace plu {
	namespace surfaces {

		box::box(vec3 c, vec3 e) {
			bounds = aabb(center-extends, center+extents);
		}

		bool box::hit(const ray& r, hit_record* hr)
		{
			vec3 _min = center - extents;
			vec3 _max = center + extents;

			vec3 rrd = 1.f / r.d;

			vec3 t1 = (_min - r.e) * rrd;
			vec3 t2 = (_max - r.e) * rrd;

			vec3 m12 = glm::min(t1, t2);
			vec3 x12 = glm::max(t1, t2);

			float tmin = m12.x;
			tmin = glm::max(tmin, m12.y);
			tmin = glm::max(tmin, m12.z);

			float tmax = x12.x;
			tmax = glm::min(tmax, x12.y);
			tmax = glm::min(tmax, x12.z);

			if (tmax < tmin || tmin < 0 || tmin > hr->t) return false;
			hr->hit_surface = this;
			hr->t = tmin;
			vec3 p = r(tmin);
			hr->normal = get_normal(p);
			hr->texture_coords.x = p.x;
			hr->texture_coords.y = p.z;
			return true;
		}

		vec3 box::get_normal(vec3 p)
		{
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