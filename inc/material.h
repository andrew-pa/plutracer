#pragma once
#include "cmmn.h"
#include "memory.h"
#include "props.h"
#include "sampler.h"

namespace plu {
	namespace detail {
		inline bool same_hemi(vec3 a, vec3 b) {
			return a.z*b.z > 0.f;
		}
		inline float cos_theta(vec3 v) { return v.z; }
		inline float sin_theta_sq(vec3 v) { return glm::max(0.f, 1.f - cos_theta(v)*cos_theta(v)); }
		inline float sin_theta(vec3 v) { return sqrt(sin_theta_sq(v)); }

		inline float cos_phi(vec3 v) {
			float st = sin_theta(v);
			if (st == 0.f) return 1.f;
			return clamp(v.x / st, -1.f, 1.f);
		}
		inline float sin_phi(vec3 v) {
			float st = sin_theta(v);
			if (st == 0.f) return 0.f;
			return clamp(v.y / st, -1.f, 1.f);
		}
	}

	struct bxdf {
		const enum type {
			reflection			= 1<<0,
			transmission		= 1<<1,
			diffuse				= 1<<2,
			glossy				= 1<<3,
			specular			= 1<<4,
			all_types			= diffuse|glossy|specular,
			all_reflection		= reflection|all_types,
			all_transmission	= transmission|all_types,
			all					= all_reflection|all_transmission
		} type_;
		
		inline bool matches_type(type T) const {
			return (type_ & T) == type_;
		}

		virtual vec3 F(vec3 wo, vec3 wi) const = 0;
		virtual float pdf(vec3 wo, vec3 wi) const {
			return detail::same_hemi(wo,wi) ? abs(detail::cos_theta(wi)) * one_over_pi<float>() : 0.f; 
		}

		virtual vec3 sampleF(vec3 wo, vec3* wi, vec2 u, float* pdf_) const { 
			*wi = rnd::cosine_hemisphere_sample(u);
			if(wo.z < 0.f) wi->z *= -1.f;
			*pdf_ = pdf(wo, *wi);
			return F(wo, *wi);
		}

		virtual ~bxdf() {}
	protected:
		bxdf(type T) : type_(T) {}
	};


	struct fresnel { virtual vec3 operator()(float cos_i) const = 0; virtual ~fresnel() {} };

	namespace bxdfs {
		struct lambert_brdf : public bxdf {
			vec3 R;
			lambert_brdf(vec3 r) : R(r), bxdf(type(reflection | diffuse)) {}

			vec3 F(vec3 wo, vec3 wi) const override {
				return R * one_over_pi<float>();
			}
		};

	}

	struct bsdf {
		size_t num_components;
		bxdf* components[8];
		vec3 S, T, N;
		bsdf(vector<bxdf*> l) : num_components(l.size()) {
			assert(l.size() <= 8);
			size_t i = 0;
			for (auto x : l) {
				components[i++] = x;
			}
		}
		bsdf(initializer_list<bxdf*> l) : bsdf(vector<bxdf*>(l.begin(), l.end())) {}
		inline bsdf& complete(const hit_record& hr) {
			N = hr.normal;
			S = normalize(hr.dpdu);
			T = cross(N, S);
			return *this;
		}
		inline size_t count_matching_components(bxdf::type t) const {
			size_t n = 0;
			for (size_t i = 0; i < num_components; ++i)
				if (components[i]->matches_type(t)) n++;
			return n;
		}

		inline vec3 w2l(vec3 v) const {
			return vec3(dot(v, S), dot(v, T), dot(v, N));
		}
		inline vec3 l2w(vec3 v) const {
			return vec3(
				S.x*v.x + T.x*v.y + N.x*v.z,
				S.y*v.x + T.y*v.y + N.y*v.z,
				S.z*v.x + T.z*v.y + N.z*v.z);
		}
		vec3 F(vec3 n, vec3 wwo, vec3 wwi, bxdf::type types) const;
		vec3 sampleF(sample& smp, vec3 n, vec3 wwo, vec3* wwi, float* pdf, bxdf::type types, bxdf::type* sampled_type = nullptr) const;
		float pdf(vec3 wo, vec3 wi, bxdf::type t) const;
	};

	struct material {

		virtual bsdf operator()(const hit_record& hr, memory::arena& ma) const = 0;
		virtual struct light* area_light() const { return nullptr; }
		virtual vec3 Le(vec3 p, vec3 n, vec3 v) const {
			auto l = area_light();
			return vec3(0.f);//l ? l->L(p, n, v) : vec3(0.f);
		}
	};

	namespace materials {
		struct diffuse_material : public material {
			props::color diffuse;
			diffuse_material(props::color df) : diffuse(df) {}

			bsdf operator()(const hit_record& hr, memory::arena& ma) const {
				return bsdf{
					new (ma.allocate(sizeof(bxdfs::lambert_brdf))) bxdfs::lambert_brdf(diffuse(hr))
				}.complete(hr);
			}
		};
	}
}
