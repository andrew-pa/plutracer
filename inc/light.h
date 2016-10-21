#pragma once
#include "cmmn.h"
#include "sampler.h"

namespace plu {
	struct light {
		virtual float pdf(vec3 p, vec3 wi) const = 0;
		virtual vec3 sampleL(vec3 p, sample& smp, vec3* wi, float* pdf, ray* v) const = 0;
		virtual vec3 sampleL(vec3 p, sample& smp, ray* r, vec3* n, float* pdf) const = 0;
		virtual bool is_delta() const = 0;
		virtual vec3 Le(const ray& r) { return vec3(0.f); }
	};

	namespace lights {
		struct point_light : public light {
			vec3 pos, intensity;
			point_light(vec3 p, vec3 i) : pos(p), intensity(i) {}

			float pdf(vec3 p, vec3 wi) const override { return 0.f; }
			vec3 sampleL(vec3 p, sample& smp, vec3* wi, float* pdf, ray* v) const override {
				auto l2p = pos - p;
				auto len2 = dot(l2p,l2p);
				*wi = l2p / sqrt(len2);
				*pdf = 1.f;
				*v = ray(p, *wi);
				return intensity / len2;
			}
			vec3 sampleL(vec3 p, sample& smp, ray* r, vec3* n, float* pdf) const override {
				*r = ray(p, rnd::uniform_sphere_sample(smp.next_vec2()));
				*n = normalize(r->d);
				*pdf = rnd::uniform_sphere_pdf();
				return intensity;
			}
			bool is_delta() const override { return true; }
		};
	};
}
