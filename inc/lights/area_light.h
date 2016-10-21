#pragma once
#include "cmmn.h"
#include "surface.h"
#include "light.h"

namespace plu {
	namespace lights {
		struct area_light : public light {
				virtual vec3 L(vec3 p, vec3 n, vec3 w) const = 0;
				virtual bool is_delta() const { return false; }
		};
		struct diffuse_area_light : public area_light {
			vec3 Lemit;
			const shared_ptr<surface> surf;
			const float area;

			diffuse_area_light(vec3 Le, shared_ptr<surface> surf) : Lemit(Le), surf(surf), area(surf->area()) {

			}

			vec3 L(vec3 p, vec3 n, vec3 w) const override {
				return dot(n, w) > 0.f ? Lemit : vec3(0.f);
			}

			vec3 sampleL(vec3 p, sample& smp, vec3* wi, float* pdf, ray* v) const override {
				vec3 n, ps = surf->sample(p, smp.next_vec2(), &n);
				*wi = normalize(ps-p);
				*pdf = surf->pdf(p, *wi);
				*v = ray(p, *wi);
				return L(ps,n,-*wi);
			}
			float pdf(vec3 p, vec3 wi) const override {
				return surf->pdf(p,wi);
			}
			vec3 sampleL(vec3 p, sample& smp, ray* r, vec3* n, float* _pdf) const override {
				vec3 o = surf->sample(smp.next_vec2(), n);
				vec3 d = rnd::uniform_sphere_sample(smp.next_vec2());
				if (dot(d, *n) < 0.f) d *= -1.f;
				*r = ray(o, d);
				*_pdf = surf->pdf(o) * one_over_two_pi<float>();
				return L(o, *n, d);
			}
		};
	}
	namespace materials {
		struct emission_material : public material {
			shared_ptr<lights::area_light> L;
			emission_material(shared_ptr<lights::area_light> l) : L(l) {}

			virtual lights::area_light* area_light() const { return L.get(); }
			
			virtual bsdf operator()(const hit_record & hr, memory::arena & ma) const override {
				return bsdf{}.complete(hr);
			}
		};
	}
}
