#include "material.h"

namespace plu {
	vec3 bsdf::F(vec3 n, vec3 wwo, vec3 wwi, bxdf::type types) const {
		vec3 wi = w2l(wwo), wo = w2l(wwo);
		if (dot(wwi, n) * dot(wwo, n) > 0)
			types = bxdf::type(types & ~bxdf::type::transmission);
		else
			types = bxdf::type(types & ~bxdf::type::reflection);
		vec3 f;
		for (size_t i = 0; i < num_components; ++i)
			if (components[i]->matches_type(types))
				f += components[i]->F(wo, wi);
		return f;
	}
	vec3 bsdf::sampleF(sample& smp, vec3 n, vec3 wwo, vec3* wwi, float* pdf, bxdf::type types, bxdf::type* sampled_type) const {
		//choose which bxdf to sample
		size_t matching_comps = count_matching_components(types);
		if (matching_comps == 0) {
			*pdf = 0.f;
			return vec3(0.f);
		}
		size_t which = glm::min((size_t)glm::floor(smp.next() * matching_comps), matching_comps - 1);
		bxdf* b = nullptr;
		size_t c = which;
		for (size_t i = 0; i < num_components; ++i)
			if (components[i]->matches_type(types) && c-- == 0) {
				b = components[i]; break;
			}
		//sample chosen bxdf
		vec3 wo = w2l(wwo);
		vec3 wi;
		*pdf = 0.f;
		vec3 f = b->sampleF(wo, &wi, smp.next_vec2(), pdf);
		if (*pdf == 0.f) return vec3(0.f);
		if (sampled_type) *sampled_type = b->type_;
		*wwi = l2w(wi);
		//compute overall PDF with all matching bxdfs
		if (!(b->type_ & bxdf::specular) && matching_comps > 1)
			for (size_t i = 0; i < num_components; ++i)
				if (components[i] != b && components[i]->matches_type(types))
					*pdf += components[i]->pdf(wo, wi);
		if (matching_comps > 1) *pdf /= (float)matching_comps;
		//compute value of bsdf for sampled directions
		if (!(b->type_ & bxdf::specular)) {
			f = vec3(0.f);
			if (dot(*wwi, n) * dot(wwo, n) > 0) types = bxdf::type(types & ~bxdf::transmission);
			else types = bxdf::type(types & ~bxdf::reflection);
			for (int i = 0; i < num_components; ++i)
				if (components[i]->matches_type(types))
					f += components[i]->F(wo, wi);
		}
		return f;
	}
	float bsdf::pdf(vec3 wo, vec3 wi, bxdf::type t) const {
		float pdf = 0.f; float w = 0.f;
		wo = w2l(wo); wi = w2l(wi);
		for (size_t i = 0; i < num_components; ++i)
			if (components[i]->matches_type(t)) {
				pdf += components[i]->pdf(wo, wi);
				w++;
			}
		return w > 0.f ? pdf / w : 0.f;
	}
}
