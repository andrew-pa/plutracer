#include "renderer.h"

namespace plu {
	// this code is also preliminary, mostly for testing purposes
	// there are many problems indeed here
	vec3 renderer::ray_color(const ray& r, size_t depth) const {
		hit_record hr;
		if (scene->hit(r, &hr)) {
			if (scene->hit(ray(r(hr.t+0.01f), normalize(vec3(.5f, .5f, 0.f))), nullptr)) {
				return vec3(0.f);
			}
			return hr.surf->mat->diffuse(hr);
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
		float inv_ssq = 1.f / (float)num_samples_sq;

		mutex tile_qu_mutex;
		mutex stdout_mutex;
		vector<thread> workers;
		for(int P = 0; P < thread::hardware_concurrency(); ++P) {
			workers.push_back(thread([&](){
				uint64_t tiles_rendered = 0;
				auto start = chrono::high_resolution_clock::now();
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
					render_tile(target, inv_size, inv_ssq, tile);
					tiles_rendered++;
				}
				auto end = chrono::high_resolution_clock::now();
				// unfortunatly there's really no better place to dump this information so it's just going on stdout
				// it doesn't really matter so who cares, these numbers are probably somewhat inaccurate anyways
				stdout_mutex.lock();
				cout << "thread #" << this_thread::get_id() << " rendered " << tiles_rendered << " tiles in " <<
					chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms" << endl;
				stdout_mutex.unlock();
			}));
		}

		for(auto& t : workers) t.join(); //block until render finishes for convenience
	}

	void renderer::render_tile(shared_ptr<texture2d> target, vec2 inv_size, float inv_ssq, uvec2 pos) const {
		for (uint32_t y = pos.y; y < pos.y+tile_size.y && y < target->size.y; ++y) {
			for (uint32_t x = pos.x; x < pos.x+tile_size.x && x < target->size.x; ++x) {
				//vec2 uv = (vec2(x,y) - ((vec2)target->size * .5f)) / target->size*2;
				vec2 uv = (vec2(x, y)*inv_size)*2.f - 1.f;
				vec3 col = vec3(0.f);
				for(uint32_t dy = 0; dy < num_samples_sq; ++dy)
					for (uint32_t dx = 0; dx < num_samples_sq; ++dx) {
						vec2 p = uv + ((vec2(dx, dy)*inv_ssq)*2.f - 1.f)*inv_size;
						col += ray_color(cam.generate_ray(p));
					}
				target->pixel(uvec2(x, y)) = col * inv_ssq * inv_ssq;
			}
		}
	}
}
