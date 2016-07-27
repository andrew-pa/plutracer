#pragma once
#include "cmmn.h"

namespace plu {
	class camera
	{
	public:
		vec3 pos;
		vec3 look;
		vec3 up;
		vec3 right;
		float w;

		camera(vec3 _p, vec3 target, float _w = 2.5f)
			: pos(_p), w(_w)
		{
			look = normalize(target - _p);
			right = 1.5f * normalize(cross(look, vec3(0, -1, 0)));
			up = 1.5f * normalize(cross(look, right));
		}

		inline ray generate_ray(vec2 uv) const
		{
			uv.y *= -1;
			vec3 rd = normalize(w*look + uv.x*right + uv.y*up);
			return ray(pos, rd);
		}
	};
}
