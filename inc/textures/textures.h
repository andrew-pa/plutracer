#pragma once
#include "cmmn.h"
#include "texture.h"

namespace plu {
	namespace textures {
		struct checkerboard_texture : public texture<vec3, vec2> {
			vec3 colors[2];
			float scale;

			checkerboard_texture(vec3 cA, vec3 cB, float scl) : scale(scl) {
				colors[0] = cA; colors[1] = cB;
			}

			vec3 texel(vec2 uv) const override {
				uv = floor(uv*scale);
				return colors[(size_t)mod(uv.x+uv.y,2.f)];
			}
		};

		struct grid_texture : public texture<vec3, vec2> {
			vec3 bg_color, fg_color;
			float scale, line_size;

			grid_texture(vec3 fg, vec3 bg, float s, float ls) : scale(s), line_size(ls), bg_color(bg), fg_color(fg) {}

			vec3 texel(vec2 uv) const override {
				uv = step(fract(uv*scale), vec2(line_size));
				return mix(bg_color, fg_color, glm::max(uv.x, uv.y));
			}
		};
	}
}
