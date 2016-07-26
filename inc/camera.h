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
		vec2 screen_size;
		float w;

		camera(vec3 _p, vec3 target, vec2 ss, float _w = 2.5f)
			: pos(_p), screen_size(ss), w(_w)
		{
			look = normalize(target - _p);
			right = 1.5f * normalize(cross(look, vec3(0, -1, 0)));
			up = 1.5f * normalize(cross(look, right));
		}

		inline ray generate_ray(vec2 xy) const
		{
			vec2 uv = (xy - (screen_size * .5f)) / (screen_size*2.f);
			uv.y *= -1;
			vec3 rd = normalize(w*look + uv.x*right + uv.y*up);
			return ray(pos, rd);
		}
	};
}
