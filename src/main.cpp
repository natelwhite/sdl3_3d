#include "Renderer.hpp"
#include "Materials.hpp"
#include <iostream>

Context* Context::self = 0;

int main() {

	Renderer renderer {640, 480};
	SceneMaterial mat {};

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
