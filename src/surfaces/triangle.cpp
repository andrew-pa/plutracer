#include "surfaces\triangle.h"

namespace plu {
	namespace surfaces {
		bool triangle::hit(const ray& r, hit_record* hr) const {
			float u, v;
			vec3 e1 = *vertices[1] - *vertices[0];
			vec3 e2 = *vertices[2] - *vertices[0];
			vec3 pv = cross(r.d, e2);
			float det = dot(e1, pv);
			if (det == 0)
				return false;
			float idet = 1.f / det;
			vec3 tv = r.e - *vertices[0];
			u = dot(tv, pv)*idet;
			if (u < 0 || u > 1.f)
				return false;
			vec3 qv = cross(tv, e1);
			v = dot(r.d, qv) * idet;
			if (v < 0 || u + v > 1)
				return false;
			float nt = dot(e2, qv)*idet;
			if (nt > 0 && nt < hr->t)
			{
				auto U = normalize(e1), V = normalize(e2);
				float w = 1 - (u + v);
				hr->set(this, nt, cross(U,V),//*u + *normals[1]*v + *normals[2]*w,
					*texture_coords[0]*u + *texture_coords[1]*v + *texture_coords[2]*w, U, V);
				return true;
			}
			return false;

		}

		vec3 triangle::sample(vec2 u, vec3* n) const {
			float w = 1 - (u.x + u.y);
			*n = *normals[0]*u.x + *normals[1]*u.y + *normals[2]*w;
			return *vertices[0]*u.x + *vertices[1]*u.y + *vertices[2]*w;
		}
	}
}
