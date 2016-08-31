#pragma once
#include "cmmn.h"
#include "texture.h"

namespace plu {
	//these are... special
	namespace props {
		// a color, either a solid color or a texture 
		class color {
			vec3 col;
			shared_ptr<texture<vec3, vec2>> tex;
		public:
			// create a solid color property
			color(vec3 col) : col(col), tex(nullptr) {}
			// create a solid color property, directly
			color(float R, float G, float B) : col(R,G,B), tex(nullptr) {}

			// create a texture color property
			color(shared_ptr<texture<vec3, vec2>> tex) : col(0.f), tex(tex) {}	
			// ^ it seems some what silly to only support vec2->vec3 textures but alas this is the way it is
			// 	additionally i don't think it makes much sense to support any other type, 
			// 	except perhaps a vec2->vec4 texture, but then it's not just a color prop, its a color-opacity prop

			vec3 operator() (const hit_record& hr) const {
				if(tex != nullptr) {
					return tex->texel(hr.texture_coords);
				} else {
					return col;
				}
			}
		};	
	}
}
