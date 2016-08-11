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
		//vec2 inv_size = 1.f / (vec2)target->size;
		float smp_wgt = 1.f / (float)(main_sampler->sample_count.x*main_sampler->sample_count.y);

		// obtain subsamplers that represent each tile to be rendered
		auto tiles = main_sampler->samplers_for_tiles(tile_size);
		size_t next_tile_index = 0;

		mutex tile_qu_mutex;
		mutex stdout_mutex;
		vector<thread> workers;
		for(int P = 0; P < thread::hardware_concurrency(); ++P) {
			workers.push_back(thread([&](){
				uint64_t tiles_rendered = 0;
				auto start = chrono::high_resolution_clock::now();
				vector<sample> smps; ray r;
				while(true) {
					size_t I; // acquire the index of a tile to render
					{
						tile_qu_mutex.lock();
						if (next_tile_index >= tiles.size()) {
							tile_qu_mutex.unlock();
							break;
						}
						I = next_tile_index++;
						tile_qu_mutex.unlock();
					}

					// loop through all of this tile's samples
					while (tiles[I]->generate_samples(smps)) {
						for (const auto& s : smps) {
							// accumulate sample colors
							cam.generate_ray(r, s);
							target->pixel(floor(s.px)) += ray_color(r) * smp_wgt;
						}
					}

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
}
