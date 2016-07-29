
#include "cmmn.h"
#include "texture.h"
#include "renderer.h"
#include "surfaces/surfaces.h"
#include "textures/textures.h"

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

int main(int argc, char* argv[]) {
	vector<string> args; for (int i = 1; i < argc; ++i) args.push_back(argv[i]);

	//**** this code is temporary, just for an example ****
	// this code is actually terrible please don't replicate this
	auto init_start = chrono::high_resolution_clock::now();
	shared_ptr<plu::texture2d> tx = make_shared<plu::texture2d>(uvec2(1280, 960));
	auto m1 = make_shared<plu::material>(
		plu::props::color(make_shared<plu::textures::grid_texture>(vec3(.6f), vec3(.05f), 16.f, .1f)));
	auto m2 = make_shared<plu::material>(
		plu::props::color(make_shared<plu::textures::checkerboard_texture>(vec3(0.f,1.f,0.f), vec3(1.f,1.f,0.f), 2.f)));
	auto s = new plu::group { 
		make_shared<plu::surfaces::sphere>(vec3(-1.f, 1.2f, 0.f), 1.f, m1),
		make_shared<plu::surfaces::sphere>(vec3(1.f, 1.2f, 0.f), 1.f, m1),
		make_shared<plu::surfaces::box>(vec3(0.f), vec3(5.f, .1f, 5.f), m2)
	};
	plu::renderer r(s, plu::camera(vec3(6.f,5.f,10.f),vec3(0.f)), uvec2(64), 16);
	auto init_end = chrono::high_resolution_clock::now();

	auto render_start = chrono::high_resolution_clock::now();
	r.render(tx);
	auto render_end = chrono::high_resolution_clock::now();

	auto init_time = chrono::duration_cast<chrono::milliseconds>(init_end - init_start); // convert to milliseconds b/c humans are bad at big numbers
	auto render_time = chrono::duration_cast<chrono::milliseconds>(render_end - render_start);
	ostringstream watermark;
	watermark << "init took: " << init_time.count() << "ms" << endl
		<< "render took: " << render_time.count() << "ms" << endl;
	cout << watermark.str();

	tx->draw_text(watermark.str(), uvec2(9, 10), vec3(0.2f)); // make a snazzy drop shadow
	tx->draw_text(watermark.str(), uvec2(8, 8), vec3(1.f, 0.6f, 0)); //draw some text
	
	// generate a somewhat unique filename so that we can see our progress
	ostringstream fns;
	fns << "image_" << chrono::system_clock::now().time_since_epoch().count() << ".bmp";
	tx->write_bmp(fns.str()); //write to image.bmp
	getchar();
	return 0;
}
