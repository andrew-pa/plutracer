
#include "cmmn.h"
#include "texture.h"
#include "renderer.h"

int main(int argc, char* argv[]) {
	vector<string> args; for (int i = 1; i < argc; ++i) args.push_back(argv[i]);

	//**** this code is temporary, just for an example ****

	shared_ptr<plu::texture2d> tx = make_shared<plu::texture2d>(uvec2(512, 512));

	plu::renderer r(nullptr, {}, uvec2(64));
	r.render(tx);

	tx->draw_text("plutracer plutracer plutracer plutracer", uvec2(9, 10), vec3(0.2f)); // make a snazzy drop shadow
	tx->draw_text("plutracer plutracer plutracer plutracer", uvec2(8, 8), vec3(1.f, 0.6f, 0)); //draw some text

	tx->write_bmp("image.bmp"); //write to image.bmp

	return 0;
}
