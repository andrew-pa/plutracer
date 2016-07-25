
#include "cmmn.h"
#include "texture.h"

int main(int argc, char* argv[]) {
	vector<string> args; for (int i = 1; i < argc; ++i) args.push_back(argv[i]);

	//**** this code is temporary, just for an example ****
	plu::texture2d tx(uvec2(512, 512));

	// generate a simple texture
	for (uint32_t y = 0; y < tx.size.y; ++y) {
		for (uint32_t x = 0; x < tx.size.x; ++x) {
			vec2 uv = vec2(x, y) / (vec2)tx.size;
			tx.pixel(uvec2(x,y)) = vec3(uv.x, uv.y, 1.f-uv.x);
		}
	}

	tx.draw_text("plutracer plutracer plutracer plutracer", uvec2(9, 10), vec3(0.2f)); // make a snazzy drop shadow
	tx.draw_text("plutracer plutracer plutracer plutracer", uvec2(8, 8), vec3(1.f, 0.6f, 0)); //draw some text

	tx.write_bmp("image.bmp"); //write to image.bmp

	return 0;
}
