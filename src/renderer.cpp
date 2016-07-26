#include "renderer.h"

namespace plu {
	vec3 renderer::ray_color(const ray& r, size_t depth) const {
		hit_record hr;
		if (scene->hit(r, &hr)) {
			return abs(hr.normal);
		}
		return vec3(0.f);
	}

	void renderer::render(shared_ptr<texture2d> target) const {
		//generate a queue of tiles that need to be rendered
		queue<uvec2> tiles;
		for(uint32_t y = 0; y < target->size.y; y += tile_size.y) {
			for(uint32_t x = 0; x < target->size.x; x += tile_size.x) {
				tiles.push(uvec2(x,y));
			}
		}

		vec2 inv_size = 1.f / (vec2)target->size;

		mutex tile_qu_mutex;
		vector<thread> workers;
		for(int P = 0; P < thread::hardware_concurrency(); ++P) {
			workers.push_back(thread([&](){
				while(true) {
					uvec2 tile;
					{
						tile_qu_mutex.lock();
						if (tiles.empty()) {
							tile_qu_mutex.unlock();
							break;
						}
						tile = tiles.front(); tiles.pop();
						tile_qu_mutex.unlock();
					}
					render_tile(target, inv_size, tile);
				}		
			}));
		}

		for(auto& t : workers) t.join(); //block until render finishes for convenience
	}

	void renderer::render_tile(shared_ptr<texture2d> target, vec2 inv_size, uvec2 pos) const {
		for (uint32_t y = pos.y; y < pos.y+tile_size.y && y < target->size.y; ++y) {
			for (uint32_t x = pos.x; x < pos.x+tile_size.x && x < target->size.x; ++x) {
				vec2 uv = (vec2(x,y) - ((vec2)target->size * .5f)) / ((vec2)target->size*2.f);
				target->pixel(uvec2(x, y)) = ray_color(cam.generate_ray(uv));
			}
		}
	}
}
