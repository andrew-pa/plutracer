
#include "cmmn.h"
#include "texture.h"
#include "renderer.h"
#include "surfaces/surfaces.h"
#include "textures/textures.h"

#include "urn.h"
#include <fstream>

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
}

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

inline shared_ptr<plu::material> make_material(urn::eval_context& cx, const urn::value& v) {
	auto vs = v.get<vector<urn::value>>();
	if (vs[0].get_var() == "diffuse") {
		plu::props::color c = plu::props::color(vec3(0.f));
		if (vs[1].type == urn::value::Var) {
			if (vs[1].get_var() == "texture") {
				
				auto ts = vs[2].get<vector<urn::value>>();
				auto t = ts[0].get_var();
				if (t == "checkerboard") {
					c = plu::props::color(
						make_shared<plu::textures::checkerboard_texture>(bk2v3(cx, ts[1]), bk2v3(cx, ts[2]), cx.eval(ts[3]).get_num()));
				}
				else if (t == "grid") {
					c = plu::props::color(
						make_shared<plu::textures::grid_texture>(bk2v3(cx, ts[1]), bk2v3(cx, ts[2]), cx.eval(ts[3]).get_num(), cx.eval(ts[4]).get<double>()));
				}
				else if (t == "img") {
					c = plu::props::color(
						make_shared<plu::texture2d>(ts[1].get<string>()));
				}
				else throw;

			} else throw;
		}
		else if (vs[1].type == urn::value::Block) {
			c = plu::props::color(bk2v3(cx, vs[1]));
		}
		else throw;
		return make_shared<plu::material>(c);
	}
	else throw;
}


int main(int argc, char* argv[]) {
	vector<string> args; for (int i = 1; i < argc; ++i) args.push_back(argv[i]);

	// !! we should probably move this code somewhere that is not main, but otoh it's kinda nice to have it here
	// !! also where else would it go anyways?

	// ***** this is still preliminary, and will probably change significatly *****
	int argi = 0;

	// a little silly REPL loop so that it's easier to test urn
	// perhaps one day this will happen after it loads the scene so that you can modify that
	if(args[argi] == "/i") {
		argi++;
		urn::eval_context cx;
		cx.create_std_funcs();
		while (true) {
			string s;
			cout << "urn> ";
			getline(cin, s);
			//auto ss = istringstream(s);
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

	auto init_start = chrono::high_resolution_clock::now();

	auto ts = urn::token_stream(ifstream(args[argi]));
	urn::value scene_tlv = urn::value(ts); //requires first cmdline argument to be path to scene file for now

	auto resolu_b = scene_tlv.named_block_val("resolution");
	auto resolution = resolu_b.is_null() ? uvec2(1280, 960) : uvec2(resolu_b[0].get<int64_t>(), resolu_b[1].get<int64_t>());
	shared_ptr<plu::texture2d> tx = make_shared<plu::texture2d>( resolution );

	auto cam_b = scene_tlv.named_block_val("camera");
	auto cam = plu::camera(bk2v3(cam_b.named_block_val("position")), bk2v3(cam_b.named_block_val("target")));

	auto smp_cnt = scene_tlv.named_block_val("antialiasing-samples").get<int64_t>();
	
	map<string, shared_ptr<plu::material>> mats;
	vector<shared_ptr<plu::surface>> surfs;


	urn::eval_context cx;
	cx.create_std_funcs();

	auto matvs = scene_tlv.named_block_val("materials").get<vector<urn::value>>();
	for (int i = 0; i < matvs.size(); ++i) {
		if (matvs[i].type != urn::value::Def) throw;
		auto d = matvs[i].get<pair<string, urn::value>>();
		mats[d.first] = make_material(cx, d.second);
	}


	auto objvs = cx.eval1(scene_tlv.named_block_val("objects")).get<vector<urn::value>>();
	for (int i = 0; i < objvs.size();) {
		auto ot = objvs[i].get_var();
		if (ot == "sphere") {
			auto m = objvs[i + 3];
			auto M = m.type == urn::value::Block ? make_material(cx, m) : m.type == urn::value::Id ? mats[m.get_id()] : nullptr;
			surfs.push_back(make_shared<plu::surfaces::sphere>(bk2v3(cx, objvs[i+1]), cx.eval(objvs[i+2]).get_num(), M));
			i += 4;
		}
		else if (ot == "box") {
			auto m = objvs[i + 3];
			auto M = m.type == urn::value::Block ? make_material(cx, m) : m.type == urn::value::Id ? mats[m.get_id()] : nullptr;
			surfs.push_back(make_shared<plu::surfaces::box>(bk2v3(cx, objvs[i + 1]), bk2v3(cx, objvs[i + 2]), M));
			i += 4;
		}
	}

	auto s = //new plu::group(surfs);
		new plu::surfaces::bvh_tree(surfs);
	plu::renderer r(s, cam, uvec2(64), smp_cnt);
	
	auto init_end = chrono::high_resolution_clock::now();

	auto render_start = chrono::high_resolution_clock::now();
	r.render(tx);
	auto render_end = chrono::high_resolution_clock::now();

	auto init_time = chrono::duration_cast<chrono::milliseconds>(init_end - init_start); // convert to milliseconds b/c humans are bad at big numbers
	auto render_time = chrono::duration_cast<chrono::milliseconds>(render_end - render_start);
	ostringstream watermark;
	watermark
		<< "scene: " << args[0] << endl
		<< "init took: " << init_time.count() << "ms" << endl
		<< "render took: " << render_time.count() << "ms" << endl;
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

	delete s;

	return 0;
}
