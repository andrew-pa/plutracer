
#include "cmmn.h"
#include "texture.h"
#include "renderer.h"
#include "surfaces\surfaces.h"
#include "textures\textures.h"

int main(int argc, char* argv[]) {
	vector<string> args; for (int i = 1; i < argc; ++i) args.push_back(argv[i]);

	//**** this code is temporary, just for an example ****
	// this code is actually terrible please don't replicate this

	shared_ptr<plu::texture2d> tx = make_shared<plu::texture2d>(uvec2(512, 512));
	auto m = make_shared<plu::material>(
		plu::props::color(
			make_shared<plu::textures::grid_texture>(vec3(0.f), vec3(1.f), 16.f, .1f)));
	auto s = make_shared<plu::surfaces::sphere>(vec3(0.f), 1.f, m);
	plu::renderer r(s, plu::camera(vec3(0.f,0.f,10.f),vec3(0.f)), uvec2(64));
	r.render(tx);

	tx->draw_text("plutracer plutracer plutracer plutracer", uvec2(9, 10), vec3(0.2f)); // make a snazzy drop shadow
	tx->draw_text("plutracer plutracer plutracer plutracer", uvec2(8, 8), vec3(1.f, 0.6f, 0)); //draw some text
	
	// generate a somewhat unique filename so that we can see our progress
	ostringstream fns;
	fns << "image_" << chrono::system_clock::now().time_since_epoch().count() << ".bmp";
	tx->write_bmp(fns.str()); //write to image.bmp

	return 0;
}
