#pragma once
#include "cmmn.h"
#include <array>

namespace plu {
	struct sample {
		vec2 px;
		vec2 lens;
		std::array<float, 64> floats;
		size_t next_float;

		inline float next() {
			if (next_float < floats.size()) {
				return floats[next_float++];
			}
			else {
				return rnd::randf();
			}
		}
		inline vec2 next_vec2() {
			return vec2(next(), next());
		}
		inline vec3 next_vec3() {
			return vec3(next(), next(), next());
		}


		sample(vec2 image_coord = vec2(0.f)) : px(image_coord), next_float(0) {}
	};

	struct sampler {
		const uvec2 start_pos, dims, sample_count;
		uvec2 pos;

		sampler(uvec2 dims, uvec2 samples, uvec2 start) : start_pos(start), dims(dims), sample_count(samples), pos(start) {}

		virtual bool generate_samples(vector<sample>& smp) = 0;

		virtual vector<unique_ptr<sampler>> samplers_for_tiles(uvec2 tile_size) = 0;
	};

	namespace samplers {
		
		inline void stratified_sample_2d(uvec2 sample_count, bool jitter, function<void(size_t, vec2)> F) {
			size_t i = 0;
			vec2 dp = 1.f / (vec2)sample_count;
			for (size_t y = 0; y < sample_count.y; ++y)
				for (size_t x = 0; x < sample_count.x; ++x)
					F(i++, (vec2(x, y) + (jitter ? rnd::randf2()*0.999f : vec2(.5f)))*dp);
		}

		struct stratified_sampler : public sampler {
			const bool jitter_samples;

			stratified_sampler(uvec2 dims, uvec2 samples, bool jitter, uvec2 start = uvec2(0)) 
				: sampler(dims, samples, start), jitter_samples(jitter) {}

			bool generate_samples(vector<sample>& smp) override {
				if (pos.y >= start_pos.y+dims.y) return false;

				smp.resize(sample_count.x*sample_count.y);

				stratified_sample_2d(sample_count, jitter_samples, [&smp, this](size_t i, vec2 s) {
					smp[i] = sample((vec2)pos + s);
				});
				stratified_sample_2d(sample_count, jitter_samples, [&smp, this](size_t i, vec2 s) {
					smp[i].lens = s;
				});
				for(size_t i = 0; i < smp.size(); ++i)
					for (size_t j = 0; j < 64; ++j)
						smp[i].floats[j] = rnd::randf();
				

				pos.x++;
				if (pos.x > start_pos.x+dims.x) {
					pos.x = start_pos.x;
					pos.y++;
				}
				return true;
			}

			vector<unique_ptr<sampler>> samplers_for_tiles(uvec2 tile_size) {
				vector<unique_ptr<sampler>> smps;
				for (uint y = start_pos.y; y < dims.y; y += tile_size.y) {
					for (uint x = start_pos.x; x < dims.x; x += tile_size.x+1) {
						auto ts = tile_size;
						if (x + ts.x > dims.x) ts.x = dims.x - x;
						if (y + ts.y > dims.y) ts.y = dims.y - y;
						smps.push_back(make_unique<stratified_sampler>(ts, sample_count, jitter_samples, uvec2(x, y)));
					}
				}
				return smps;
			}
		};
	}
}

