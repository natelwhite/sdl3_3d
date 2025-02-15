#include "Renderer.hpp"
#include "Materials.hpp"
#include <iostream>

Context* Context::self = 0;

int main() {
	const size_t VERTEX_COUNT { 24 }, INDEX_COUNT { 36 };
	const PositionColorVertex vertices[VERTEX_COUNT] {
		{ -10, -10, -10, 255, 0, 0, 255 },
		{ 10, -10, -10, 255, 0, 0, 255 },
		{ 10, 10, -10, 255, 0, 0, 255 },
		{ -10, 10, -10, 255, 0, 0, 255 },

		{ -10, -10, 10, 255, 255, 0, 255 },
		{ 10, -10, 10, 255, 255, 0, 255 },
		{ 10, 10, 10, 255, 255, 0, 255 },
		{ -10, 10, 10, 255, 255, 0, 255 },

		{ -10, -10, -10, 255, 0, 255, 255 },
		{ -10, 10, -10, 255, 0, 255, 255 },
		{ -10, 10, 10, 255, 0, 255, 255 },
		{ -10, -10, 10, 255, 0, 255, 255 },

		{ 10, -10, -10, 0, 255, 0, 255 },
		{ 10, 10, -10, 0, 255, 0, 255 },
		{ 10, 10, 10, 0, 255, 0, 255 },
		{ 10, -10, 10, 0, 255, 0, 255 },

		{ -10, -10, -10, 255, 255, 255 },
		{ -10, -10, 10, 255, 255, 255 },
		{ 10, -10, 10, 255, 255, 255 },
		{ 10, -10, -10, 255, 255, 255 },

		{ -10, 10, -10, 0, 0, 255, 255 },
		{ -10, 10, 10, 0, 0, 255, 255 },
		{ 10, 10, 10, 0, 0, 255, 255 },
		{ 10, 10, -10, 0, 0, 255, 255 },
	};
	const Uint16 indices[INDEX_COUNT] {
		 0,  1,  2,  0,  2,  3,
		 6,  5,  4,  7,  6,  4,
		 8,  9, 10,  8, 10, 11,
		14, 13, 12, 15, 14, 12,
		16, 17, 18, 16, 18, 19,
		22, 21, 20, 23, 22, 20
	};

	Renderer renderer {640, 480};
	SceneMaterial mat {VERTEX_COUNT, INDEX_COUNT};

	// push vertices
	SDL_memcpy(mat.worldVertBuffer()->open(), vertices, sizeof(PositionColorVertex) * VERTEX_COUNT);
	mat.worldVertBuffer()->upload();
	Uint16 *data = static_cast<Uint16*>(SDL_memcpy(mat.worldIndexBuffer()->open(), indices, sizeof(Uint16) * INDEX_COUNT));
	mat.worldIndexBuffer()->upload();

	// main loop
	bool quit = false;
	while (!quit) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch(e.type) {
			case SDL_EVENT_QUIT:
				quit = true;
				break;
			case SDL_EVENT_KEY_DOWN:
				switch(e.key.key) {
				case SDLK_ESCAPE:
					quit = true;
					break;
				case SDLK_R:
					/*mat.refresh();*/
					break;
				case SDLK_W: {
					ContextData ctx { Context::get()->data() };
					ctx.camera_pos.at(2) += 1;
					Context::get()->set(ctx);
					break;
				}
				case SDLK_A: {
					ContextData ctx { Context::get()->data() };
					Vector3 cam_pos { ctx.camera_pos };
					ctx.camera_pos.at(0) -= 1;
					Context::get()->set(ctx);
					break;
				}
				case SDLK_S: {
					ContextData ctx { Context::get()->data() };
					Vector3 cam_pos { ctx.camera_pos };
					ctx.camera_pos.at(2) -= 1;
					Context::get()->set(ctx);
					break;
				}
				case SDLK_D: {
					ContextData ctx { Context::get()->data() };
					Vector3 cam_pos { ctx.camera_pos };
					ctx.camera_pos.at(0) += 1;
					Context::get()->set(ctx);
					break;
				}
				case SDLK_Z: {
					ContextData ctx { Context::get()->data() };
					Vector3 cam_pos { ctx.camera_pos };
					ctx.camera_pos.at(1) += 1;
					Context::get()->set(ctx);
					break;
				}
				case SDLK_X: {
					ContextData ctx { Context::get()->data() };
					Vector3 cam_pos { ctx.camera_pos };
					ctx.camera_pos.at(1) -= 1;
					Context::get()->set(ctx);
					break;
				}
				}
			}
		}
		mat.draw();
	}
	return 0;
}
