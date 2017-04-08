
#include "cmmn.h"
#include "texture.h"
#include "renderer.h"
#include "surfaces/surfaces.h"
#include "surfaces/triangle.h"
#include "textures/textures.h"
#include "sampler.h"
#include "light.h"
#include "lights/area_light.h"

#include "urn.h"
#include <fstream>

#include "scene.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!
!!	This entire file is sketchy and subject to change
!!	It is entierly likely that many parts of it will eventually move to 
!!	their proper place. Additionaly this scene loading code is an abomination,
!!	and needs help. Also that group surface is... sketchy.
!!	
!!	Where to eval? who knows.... this needs major help
!!
!!	RIGHT NOW THE SCRIPTING/EXECUTION PORTION OF URN IS USED, BUT NOT EVERYWHERE
!!	This is because
!!	1) I don't know what to eval and what not to, 
!!		it can't yet be deterimined where it will be useful
!!	2) Namespacing issues related to what gets eval'd and in what eval_context
!!	3) urn has no std lib, not even basic functions like + or - or even a map, 
!!		so it is basically useless. Someone needs to go and define those
!!	RIGHT NOW: Scene loader evaluates basically everything in the same evaluation context
!!		Additionally it evaluates once the entire object value
!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

namespace plu {
	// slow as possible, but for testing purposes whatever
	struct group : public plu::surface {
		vector<shared_ptr<surface>> surfaces;

		group(const vector<shared_ptr<surface>>& s) : surfaces(s) {}
		group(initializer_list<shared_ptr<surface>> s) : surfaces(s.begin(), s.end()) {}

		bool hit(const ray& r, hit_record* hr) const override {
			bool ah = false;
			hit_record mhr = hr != nullptr ? *hr : hit_record();
			for (const auto& s : surfaces) {
				hit_record phr = mhr;
				if (s->hit(r, &phr)) {
					ah = true;
					mhr = phr;
				}
			}
			if (hr != nullptr) *hr = mhr;
			return ah;
		}

		aabb bounds() const override {
			aabb tb;
			for (const auto& s : surfaces) tb.add_aabb(s->bounds());
			return tb;
		}
	};

	//this needs help
	struct postprocesser {
		vec3 whitePreservingLumaBasedReinhardToneMapping(vec3 color)
		{
			float white = 2.f;
			float luma = dot(color, vec3(0.2126f, 0.7152f, 0.0722f));
			float toneMappedLuma = luma * (1.f + luma / (white*white)) / (1.f + luma);
			color *= toneMappedLuma / luma;
			color = pow(color, vec3(1.f / 2.2f));
			return color;
		}

		void postprocess(shared_ptr<texture2d> rt) {
			vector<thread> workers; mutex scanline_mutex; uint32_t next_scanline = 0;
			for (int i = 0; i < thread::hardware_concurrency(); ++i) {
				workers.push_back(thread([&]{
					while (true) {
						uint32_t scanline;
						scanline_mutex.lock();
						{
							scanline = next_scanline++;
							if (scanline >= rt->size.y) {
								scanline_mutex.unlock();
								break;
							}
						}
						scanline_mutex.unlock();
						for (uint32_t x = 0; x < rt->size.x; ++x) {
							auto c = rt->pixel(uvec2(x, scanline));
							rt->pixel(uvec2(x, scanline)) = whitePreservingLumaBasedReinhardToneMapping(c);
						}
					}
				}));
			}
			for (auto& t : workers) t.join();
		}
	};
}

int main(int argc, char* argv[]) {
	list<string> args; for (int i = 1; i < argc; ++i) args.push_back(argv[i]);

	// !! we should probably move this code somewhere that is not main, but otoh it's kinda nice to have it here
	// !! also where else would it go anyways?

	// ***** this is still preliminary, and will probably change significatly *****

	// a little silly REPL loop so that it's easier to test urn
	// perhaps one day this will happen after it loads the scene so that you can modify that
	if(args.front() == "/i") {
		args.pop_front();
		urn::eval_context cx;
		cx.create_std_funcs();
		while (true) {
			string s;
			cout << "urn> ";
			getline(cin, s);
			auto ts = urn::token_stream(istringstream(s));
			auto v = urn::value(ts);
			if (v.type == urn::value::Val) {
				auto cmd = v.get_val();
				if (cmd == "!q") break;
				else if (cmd == "!x") return 42;
			}
			cout << cx.eval(v) << endl;
		}
	}

	auto scn_path = args.front(); args.pop_front();
	cout << "loading scene " << scn_path << endl;
	auto init_start = chrono::high_resolution_clock::now();

	auto ts = urn::token_stream(ifstream(scn_path));
	urn::value scene_tlv = urn::value(ts); //requires first cmdline argument to be path to scene file for now

	plu::scenes::scene sc{scene_tlv, args};

	shared_ptr<plu::texture2d> tx = make_shared<plu::texture2d>(sc.resolution);

/*	auto M = make_shared<plu::materials::emission_material>(nullptr);
	auto sh = //make_shared<plu::surfaces::sphere>(vec3(-1.f, 4.9f, -1.f), 1.f, M);
				make_shared<plu::surfaces::box>(vec3(-1.f, 4.9f, -1.f), vec3(1.f, .1f, 1.f), M);
	auto L = make_shared<plu::lights::diffuse_area_light>(vec3(10000000.f), sh);
	M->L = L;*/
	//lights.push_back(L);
	//surfs.push_back(sh);
	/*vec3 a(0.f, 1.f, -3.f), b(3.f, 1.f, 3.f), c(-3.f, 1.f, 3.f), n(0.f, 1.f, 0.f);
	vec2 t(0.f);
	auto tri = make_shared<plu::surfaces::triangle>(&a, &b, &c, &n, &n, &n, &t, &t, &t);
	tri->mat = make_shared<plu::materials::diffuse_material>(vec3(.6f, 0.f, 0.f));
	sc.surfs.push_back(tri);*/

	auto s = //new plu::group(surfs);
		new plu::surfaces::bvh_tree(sc.surfs);
	auto sampler = new plu::samplers::stratified_sampler(tx->size, uvec2(sc.samples), true);
	plu::renderer r(s, sc.cam, uvec2(32), sampler, sc.lights);
	
	auto init_end = chrono::high_resolution_clock::now();

	cout << "rendering... " << endl;
	auto render_start = chrono::high_resolution_clock::now();
	r.render(tx);
	auto render_end = chrono::high_resolution_clock::now();

	cout << "postprocessing... " << endl;
	auto pp_start = chrono::high_resolution_clock::now();
	plu::postprocesser pp;
	pp.postprocess(tx);
	auto pp_end = chrono::high_resolution_clock::now();
	cout << "... finished" << endl;

	auto init_time = chrono::duration_cast<chrono::milliseconds>(init_end - init_start); // convert to milliseconds b/c humans are bad at big numbers
	auto render_time = chrono::duration_cast<chrono::milliseconds>(render_end - render_start);
	auto pp_time = chrono::duration_cast<chrono::milliseconds>(pp_end - pp_start);
	ostringstream watermark;
	watermark
		<< "scene: " << scn_path << endl
		<< "init took: " << init_time.count() << "ms" << endl
		<< "render took: " << render_time.count() << "ms" << endl
		<< "postprocess took: " << pp_time.count() << "ms" << endl;
#ifdef _DEBUG
	watermark << "DEBUG" << endl;
#else
	watermark << "RELEASE" << endl;
#endif
	cout << watermark.str();

	tx->draw_text(watermark.str(), uvec2(9, 10), vec3(0.2f)); // make a snazzy drop shadow
	tx->draw_text(watermark.str(), uvec2(8, 8), vec3(1.f, 0.6f, 0)); //draw some text
	
	// generate a somewhat unique filename so that we can see our progress
	ostringstream fns;
	fns << "image_" << chrono::system_clock::now().time_since_epoch().count() << ".bmp";
	tx->write_bmp(fns.str()); //write to image.bmp
	getchar();

	delete s, sampler;

	return 0;
}
