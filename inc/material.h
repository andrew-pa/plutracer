#pragma once
#include "cmmn.h"
#include "memory.h"
#include "props.h"
#include "sampler.h"

namespace plu {
	namespace util {
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

		inline vec3 fresnel_dieletric(float cos_i, float cos_t, vec3 etai, vec3 etat) {
			vec3 Rparl = 
				((etat * cos_i) - (etai*cos_t)) /
				((etat * cos_i) + (etai*cos_t));
			vec3 Rperp =
				((etai*cos_i) - (etat*cos_t)) /
				((etai*cos_i) + (etat*cos_t));
			return (Rparl*Rparl + Rperp*Rperp) * 0.5f;
		}
		inline vec3 fresnel_conductor(float cos_i, vec3 eta, vec3 k) {
			vec3 tmp0 = (eta*eta + k*k);
			vec3 tmp1 = tmp0 * cos_i*cos_i;
			vec3 Rparl2 =
				(tmp1 - (2.f*eta*cos_i) + 1.f) /
				(tmp1 + (2.f*eta*cos_i) + 1.f);
			vec3 Rperp2 =
				(tmp0 - (2.f*eta*cos_i) + cos_i*cos_i) /
				(tmp0 - (2.f*eta*cos_i) + cos_i*cos_i);
			return (Rparl2 + Rperp2) / 2.f;
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
			return util::same_hemi(wo,wi) ? abs(util::cos_theta(wi)) * one_over_pi<float>() : 0.f; 
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
	struct fresnel_conductor : public fresnel {
		vec3 eta, k;
		fresnel_conductor(vec3 e, vec3 k) : eta(e), k(k) {}
		vec3 operator()(float cos_i) const override {
			return util::fresnel_conductor(abs(cos_i), eta, k);
		}
	};
	struct fresnel_dielectric : public fresnel {
		float eta_i, eta_t;
		fresnel_dielectric(float i, float t) : eta_i(i), eta_t(t) {}
		vec3 operator()(float cos_i) const override {
			cos_i = clamp(cos_i, -1.f, 1.f);
			bool entering = cos_i > 0.f;
			float ei = eta_i, et = eta_t;
			if (entering) swap(ei, et);
			float sin_t = ei / et * sqrt(glm::max(0.f, 1.f - cos_i*cos_i));
			if (sin_t >= 1.f) return vec3(1.f);
			else {
				float cos_t = sqrt(glm::max(0.f, 1.f - sin_t*sin_t));
				return util::fresnel_dieletric(abs(cos_i), cos_t, vec3(ei), vec3(et));
			}
		}
	};
	

	namespace bxdfs {
		struct lambert_brdf : public bxdf {
			vec3 R;
			lambert_brdf(vec3 r) : R(r), bxdf(type(reflection | diffuse)) {}

			vec3 F(vec3 wo, vec3 wi) const override {
				return R * one_over_pi<float>();
			}
		};
		struct specular_reflection : public bxdf {
			vec3 R; fresnel* Fr;
			specular_reflection(vec3 r, fresnel* f) : bxdf(type(reflection|specular)), R(r), Fr(f) {}
			vec3 F(vec3, vec3) const override { return vec3(0.f); }
			float pdf(vec3, vec3) const override { return 0.f; }
			vec3 sampleF(vec3 wo, vec3* wi, vec2 u, float* pdf_) const {
				*wi = vec3(-wo.x, -wo.y, wo.z);
				*pdf_ = 1.f;
				return (*Fr)(util::cos_theta(wo)) * R / abs(util::cos_theta(*wi));
			}
		};
		struct specular_transmission : public bxdf {
			vec3 T;
			fresnel_dielectric Fr;
			specular_transmission(vec3 t, float et, float ei) : T(t), Fr(et, ei), bxdf(type(transmission | specular)) {}
			vec3 F(vec3, vec3) const override { return vec3(0.f); }
			float pdf(vec3, vec3) const override { return 0.f; }
			vec3 sampleF(vec3 wo, vec3* wi, vec2 u, float* pdf_) const {
				bool entering = util::cos_theta(wo) > 0.f;
				float ei = Fr.eta_i, et = Fr.eta_t;
				if (!entering) swap(ei, et);
				float sin2_i = util::sin_theta_sq(wo);
				float eta = ei / et;
				float sin2_t = eta*eta*sin2_i;
				if (sin2_t >= 1.f) return vec3(0.f);
				float cos_t = sqrt(glm::max(0.f, 1.f - sin2_t));
				if (entering) cos_t *= -1.f;
				*wi = vec3(eta*-wo.x, eta*-wo.y, cos_t);
				*pdf_ = 1.f;
				return (et*et) / (ei*ei) *
					(1.f - Fr(util::cos_theta(wo))) * T / abs(util::cos_theta(*wi));
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
			N = hr.norm;
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
	
	namespace lights { struct area_light; }
	struct material {

		virtual bsdf operator()(const hit_record& hr, memory::arena& ma) const = 0;
		virtual lights::area_light* area_light() const { return nullptr; }
		vec3 Le(vec3 p, vec3 n, vec3 v) const;
	};

	namespace materials {
#define newma(type) new (ma.allocate(sizeof(type))) type
		struct diffuse_material : public material {
			props::color diffuse;
			diffuse_material(props::color df) : diffuse(df) {}

			bsdf operator()(const hit_record& hr, memory::arena& ma) const {
				return bsdf{
					newma(bxdfs::lambert_brdf)(diffuse(hr))
				}.complete(hr);
			}
		};
		struct perfect_reflection_material : public material {
			props::color color; vec3 eta, k;
			perfect_reflection_material(props::color c, vec3 eta, vec3 k) : color(c), eta(eta), k(k) {}

			bsdf operator()(const hit_record& hr, memory::arena& ma) const {
				return bsdf{
					newma(bxdfs::specular_reflection)(color(hr), 
						newma(fresnel_conductor)(eta,k) )
				}.complete(hr);
			}
		};
		struct perfect_refraction_material : public material {
			props::color color; float eta_t, eta_i;
			perfect_refraction_material(props::color c, float t, float i) : color(c), eta_t(t), eta_i(i) {}

			bsdf operator()(const hit_record& hr, memory::arena& ma) const {
				return bsdf{
					newma(bxdfs::specular_transmission)(color(hr), eta_t, eta_i)
				}.complete(hr);
			}
		};
		struct glass_material : public material {
			props::color color; float ior;
			glass_material(props::color c, float R) : color(c), ior(R) {}
			bsdf operator()(const hit_record& hr, memory::arena& ma) const {
				auto c = color(hr);
				return bsdf{
					newma(bxdfs::specular_reflection)(c, newma(fresnel_dielectric)(1.f, ior)),
					newma(bxdfs::specular_transmission)(c, 1.f, ior)
				}.complete(hr);
			}
		};
#undef newma
	}
}
