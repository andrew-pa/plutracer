#pragma once
#include "cmmn.h"
#include "surface.h"
#include "surfaces\surfaces.h"
#include "light.h"
#include "lights\area_light.h"
#include "material.h"
#include "camera.h"
#include "texture.h"
#include "textures\textures.h"

#include "urn.h"

namespace plu {
	namespace scenes {
		inline vec3 bk2v3(const urn::value& v) {
			return vec3(v[0].get_num(), v[1].get_num(), v[2].get_num());
		}

		inline vec3 bk2v3(urn::eval_context& cx, const urn::value& v) {
			auto rv = cx.reduce(v);
			return vec3(rv[0].get_num(), rv[1].get_num(), rv[2].get_num());
		}

		/*
		Example of a scene definition:

		resolution: [1280 960]
		antialiasing: 16
		camera: [
			position:	[0 0 -5]
			target:		[0 0 0]
			up:			[0 1 0]
		]
		materials: [
			green: [ diffuse [0.1 0.7 0.2] ]
			red: [ diffuse [0.7 0.2 0.1] ]
			checkerboard: [ diffuse texture [ checkerboard [0 0 0] [1 1 1] 8 ] ]
		]
		objects: do [
			concat [
				box [0 0 0] [10 0.1 10] 'checkerboard
			] [
				sphere [1.4 1.1 0] 1 'green
				sphere [-1.4 1.1 0] 1 'red
			]
		]
		*/

		/*
			+ more lights
				- area lights
				- inf area lights
			+ more materials
				- Cook-Torrance
				- Ashikhmin&Shirley
				- Disney
			+ real stratified samples
			+ fix the box so that it can be used on all sides
			+ speed
		*/
		struct scene {
			uvec2 resolution; uint64 samples;
			camera cam;
			map<string, shared_ptr<material>> mats;
			vector<shared_ptr<surface>> surfs;
			vector<shared_ptr<light>> lights;

			inline props::color make_color(urn::eval_context& cx, const vector<urn::value>& vs, int& i) {
				if (vs[i].type == urn::value::Var) {
					if (vs[i].get_var() == "texture") {
						i++;
						auto ts = vs[i].get<vector<urn::value>>(); i++;
						auto t = ts[0].get_var();
						if (t == "checkerboard") {
							return props::color(
								make_shared<textures::checkerboard_texture>(bk2v3(cx, ts[1]), bk2v3(cx, ts[2]), cx.eval(ts[3]).get_num()));
						}
						else if (t == "grid") {
							return props::color(
								make_shared<textures::grid_texture>(bk2v3(cx, ts[1]), bk2v3(cx, ts[2]), cx.eval(ts[3]).get_num(), cx.eval(ts[4]).get<double>()));
						}
						else if (t == "img") {
							return props::color(
								make_shared<texture2d>(ts[1].get<string>()));
						}
						else throw;

					}
					else throw;
				}
				else if (vs[i].type == urn::value::Block) {
					return props::color(bk2v3(cx, vs[i++]));
				}
				else throw;
			}

			inline shared_ptr<material> make_material(urn::eval_context& cx, const urn::value& v) {
				auto vs = v.get<vector<urn::value>>();
				if (vs[0].get_var() == "diffuse") {
					int i = 1;
					return make_shared<materials::diffuse_material>(make_color(cx, vs, i));
				}
				else if (vs[0].get_var() == "perfect-reflection") {
					int i = 1;
					auto c = make_color(cx, vs, i);
					auto e = bk2v3(cx, vs[i++]), k = bk2v3(cx, vs[i++]);
					return make_shared<materials::perfect_reflection_material>(c, e, k);
				}
				else if (vs[0].get_var() == "perfect-refraction") {
					int i = 1;
					auto c = make_color(cx, vs, i);
					return make_shared<materials::perfect_refraction_material>(c, vs[i].get_num(), vs[i + 1].get_num());
				}
				else if (vs[0].get_var() == "glass") {
					int i = 1;
					auto c = make_color(cx, vs, i);
					return make_shared<materials::glass_material>(c, vs[i].get_num());
				}
				else throw;
			}

			inline shared_ptr<plu::material> make_or_ref_material(urn::eval_context& cx, urn::value m) {
				return m.type == urn::value::Block ? make_material(cx, m) : m.type == urn::value::Id ? mats[m.get_id()] : nullptr;
			}

			inline shared_ptr<surface> make_basic_surface(urn::eval_context& cx, vector<urn::value>& objvs, int& i) {
				auto ot = objvs[i].get_var();
				if (ot == "sphere") {
					auto s = make_shared<surfaces::sphere>(
						bk2v3(cx, objvs[i + 1]), 
						cx.eval(objvs[i + 2]).get_num(), 
						nullptr);
					i += 3;
					return s;
				}
				else if (ot == "box") {
					auto s = make_shared<surfaces::box>(
						bk2v3(cx, objvs[i + 1]), 
						bk2v3(cx, objvs[i + 2]), 
						nullptr);
					i += 3;
					return s;
				}
				else return nullptr;
			}


			scene(urn::value scene_top_level_value, list<string>& args) {
				auto resolu_b = scene_top_level_value.named_block_val("resolution");
				resolution = resolu_b.is_null() ? uvec2(1280, 960) : uvec2(resolu_b[0].get<int64_t>(), resolu_b[1].get<int64_t>());
				auto res_override_flag = find(args.begin(), args.end(), "/res");
				if (res_override_flag != args.end()) {
					auto resstr = *(++res_override_flag);
					args.remove("/res"); args.remove(resstr);
					auto xloc = resstr.find('x');
					resolution = uvec2(atoi(resstr.substr(0, xloc).c_str()), atoi(resstr.substr(xloc+1).c_str()));
				}

				auto cam_b = scene_top_level_value.named_block_val("camera");
				vec2 dof_settings;
				if (cam_b.has_block_val_named("lens")) {
					auto lens_b = cam_b.named_block_val("lens");
					dof_settings.x = lens_b.named_block_val("radius").get_num();
					dof_settings.y = lens_b.named_block_val("focal-distance").get_num();
				}
				cam = camera(bk2v3(cam_b.named_block_val("position")), bk2v3(cam_b.named_block_val("target")),
					resolution, dof_settings.x, dof_settings.y);

				samples = scene_top_level_value.named_block_val("antialiasing-samples").get<int64_t>();
				auto samples_override_flag = find(args.begin(), args.end(), "/smp");
				if (samples_override_flag != args.end()) {
					auto smpstr = *(++samples_override_flag);
					args.remove("/smp"); args.remove(smpstr);
					samples = atoll(smpstr.c_str());
				}

				urn::eval_context cx;
				cx.create_std_funcs();

				auto matvs = scene_top_level_value.named_block_val("materials").get<vector<urn::value>>();
				for (int i = 0; i < matvs.size(); ++i) {
					if (matvs[i].type != urn::value::Def) throw;
					auto d = matvs[i].get<pair<string, urn::value>>();
					mats[d.first] = make_material(cx, d.second);
				}


				auto objvs = cx.eval1(scene_top_level_value.named_block_val("objects")).get<vector<urn::value>>();
				for (int i = 0; i < objvs.size();) {
					/*if (ot == "sphere") {
						auto m = objvs[i + 3];
						auto M = m.type == urn::value::Block ? make_material(cx, m) : m.type == urn::value::Id ? mats[m.get_id()] : nullptr;
						surfs.push_back(make_shared<surfaces::sphere>(bk2v3(cx, objvs[i + 1]), cx.eval(objvs[i + 2]).get_num(), M));
						i += 4;
					}
					else if (ot == "box") {
						auto m = objvs[i + 3];
						auto M = m.type == urn::value::Block ? make_material(cx, m) : m.type == urn::value::Id ? mats[m.get_id()] : nullptr;
						surfs.push_back(make_shared<surfaces::box>(bk2v3(cx, objvs[i + 1]), bk2v3(cx, objvs[i + 2]), M));
						i += 4;
					}
					else*/
					auto bo = make_basic_surface(cx, objvs, i);
					if (bo == nullptr) {
						auto ot = objvs[i].get_var();
						if (ot == "point-light") {
							lights.push_back(make_shared<lights::point_light>(bk2v3(cx, objvs[i + 1]), bk2v3(cx, objvs[i + 2])));
							i += 3;
						}
						else if (ot == "diffuse-area-light") {
							auto M = make_shared<plu::materials::emission_material>(nullptr);
							int j = 0;
							auto s = make_basic_surface(cx, cx.eval1(objvs[i + 1]).get<vector<urn::value>>(), j);
							s->mat = M;
							auto L = make_shared<lights::diffuse_area_light>(bk2v3(cx, objvs[i + 2]), s);
							M->L = L;
							lights.push_back(L);
							surfs.push_back(s);
							i += 3;
						}
					}
					else {
						bo->mat = make_or_ref_material(cx, objvs[i]);
						i++;

						surfs.push_back(bo);
					}
				}
			}
		};
	}
}
