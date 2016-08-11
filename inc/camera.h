 #pragma once
#include "cmmn.h"
#include "sampler.h"

namespace plu {
	class camera
	{
	public:
		vec3 pos;
		vec3 look;
		vec3 up;
		vec3 right;
		vec2 inv_image_size;
		float w, lens_radius, focal_distance;

		camera(vec3 _p, vec3 target, uvec2 target_size, float ln = 0.f, float fd = 0.f, float _w = 2.5f)
			: pos(_p), w(_w), lens_radius(ln), focal_distance(fd), inv_image_size(1.f/(vec2)target_size)
		{
			look = normalize(target - _p);
			right = 1.5f * normalize(cross(look, vec3(0, -1, 0)));
			up = 1.5f * normalize(cross(look, right));
		}

		inline void generate_ray(ray& r, const sample& smp) const
		{
			vec2 uv = (smp.px * inv_image_size) * 2.f - 1.f;
			uv.y *= -1;
			r.e = pos;
			r.d = normalize(w*look + uv.x*right + uv.y*up);
			if (lens_radius > 0.f) {
				vec2 l = rnd::concentric_disk_map(smp.lens)*lens_radius;
				vec3 pof = r(focal_distance / r.d.z);
				r.e.xy += l;
				r.d = normalize(pof - r.e);
			}
		}
	};

	/*
	poor attempt at a camera that uses matrices to generate rays, couldn't fix it
	struct matrix_camera{
		mat4 view;
		mat4 projection;
		float lens_radius, focal_distance;


		badcamera(const mat4& viewT, float fov, uvec2 target_size, float lensR = 0.f, float focalD = 0.f) : view(viewT), lens_radius(lensR), focal_distance(focalD) {
			auto cam2screen = perspectiveLH(fov, 1.f, 0.01f, 10000.f);
			auto screen2raster = scale(mat4(1), vec3((vec2)target_size, 1.f));
			projection = inverse(screen2raster) * inverse(cam2screen);
		}

		inline void generate_ray(ray& out, const sample& smp) const {
			vec3 d = (projection*vec4(smp.px, 0.f, 1.f)).xyz;
			out = ray(vec3(0.f), normalize(d));
			if (lens_radius > 0.f) {
				vec2 l = rnd::concentric_disk_map(smp.lens)*lens_radius;
				vec3 focusP = out(focal_distance / out.d.z);
				out.e = vec3(l, 0.f);
				out.d = normalize(focusP - out.e);
			}
			out.transform(view);
		}
	};
	*/
}
