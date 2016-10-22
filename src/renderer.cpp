#include "renderer.h"

namespace plu {

	vec3 renderer::estimate_direct_light(sample& smp, vec3 p, vec3 n, vec3 wo, bsdf& b, light* l, bxdf::type t) const {
		vec3 Ld;
		//sample light source with multipule importance sampling
		vec3 wi; float light_pdf, bsdf_pdf; ray r;
		vec3 Li = l->sampleL(p, smp, &wi, &light_pdf, &r);
		if (light_pdf > 0.f && dot(Li, Li) > 0.f) {
			//if (b.num_components > 0)
			//	cout << "";
			vec3 f = b.F(n, wo, wi, t);
			if (dot(f, f) > 0.f) {
				hit_record light_hr;
				bool intersection_on_wi = scene->hit(r, &light_hr);
				if (!intersection_on_wi || (!l->is_delta() && (light*)light_hr.surf->mat->area_light() == l)) {
					//add light's contribution to reflected radiance
					if (l->is_delta()) Ld += f * Li * (abs(dot(wi, n)) / light_pdf);
					else {
						bsdf_pdf = b.pdf(wo, wi, t);
						float w = (bsdf_pdf*bsdf_pdf) / (bsdf_pdf*bsdf_pdf + light_pdf*light_pdf);
						Ld += f * Li * (abs(dot(wi, n))*w / light_pdf);
					}
				}
			}
		}
		//sample BSDF with multipule importance sampling
		if (!l->is_delta()) {
			bxdf::type sampled_type;
			vec3 f = b.sampleF(smp, n, wo, &wi, &bsdf_pdf, t, &sampled_type);
			if (dot(f, f) > 0.f && bsdf_pdf > 0.f) {
				float w = 1.f;
				if (!(sampled_type & bxdf::specular)) {
					light_pdf = l->pdf(p, wi);
					if (light_pdf == 0.f) return Ld;
					w = (bsdf_pdf*bsdf_pdf) / (bsdf_pdf*bsdf_pdf + light_pdf*light_pdf);
				}
				//add light contribution from BSDF sampling
				hit_record lihr; r = ray(p, wi);
				if (scene->hit(r, &lihr)) {
					if ((light*)lihr.surf->mat->area_light() == l) Li = lihr.surf->mat->Le(p,n,-wi);
					else Li = l->Le(-wi);
					if (dot(Li, Li) > 0.f) {
						Ld += f * Li * abs(dot(wi, n)) * w / bsdf_pdf;
					}
				}
			}
		}
		return Ld;
	}

	vec3 renderer::uniform_sample_one_light(sample& smp, vec3 p, vec3 n, vec3 wo, bsdf& b) const {
		//pick the light
		auto l = lights[(size_t)glm::min(floor(smp.next()*(float)lights.size()),(float)lights.size()-1.f)];
		return (float)lights.size() * estimate_direct_light(smp, p, n, wo, b, l.get(), bxdf::type(bxdf::all & ~bxdf::specular));
	}

	vec3 renderer::ray_color(const ray& _r, sample& smp, memory::arena& arena) const {
		hit_record hr;
		if (scene->hit(_r, &hr)) {
			vec3 path_throughput = vec3(1.f), L = vec3(0.f);
			bool spec_bounce = false; ray r = _r;
			for (int bounces = 0;; bounces++) {
				//add emited light at vertex
				if (bounces == 0 || spec_bounce) L += path_throughput * hr.surf->mat->Le(r(hr.t),hr.norm,-r.d);
				//sample illumination from lights to find path contribution
				auto bsdf = (*hr.surf->mat)(hr, arena);
				//if (bsdf.num_components > 0)
				//	int i = 0;
				const vec3& p = r(hr.t);
				const vec3& n = hr.norm;
				vec3 wo = -r.d;
				L += path_throughput * uniform_sample_one_light(smp, p, n, -r.d, bsdf);
				//sample BSDF to get new path direction
				vec3 wi; float pdf; bxdf::type types;
				vec3 f = bsdf.sampleF(smp, n, wo, &wi, &pdf, bxdf::all, &types);
				if (dot(f, f) == 0.f || pdf == 0.f) break;
				spec_bounce = (types & bxdf::specular) != 0;
				path_throughput *= f * abs(dot(wi, n)) / pdf;
				r = ray(p, wi);
				//terminate path?
				if (bounces > 6)
					break;
				//find next path vertex
				if (!scene->hit(r, &hr)) {
					if (spec_bounce)
						for (const auto& l : lights)
							L += path_throughput * l->Le(r);
					break;
				}
			}
			return L;
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
				memory::arena arena;
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
						for (auto& s : smps) {
							// accumulate sample colors
							cam.generate_ray(r, s);
							target->pixel(floor(s.px)) += ray_color(r, s, arena) * smp_wgt;
						}
					}
					cout << ".";
					tiles_rendered++;
					arena.free_all();
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
