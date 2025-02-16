#include "Renderer.hpp"
#include "Materials.hpp"

Context* Context::self = 0;

int main() {

	Renderer renderer {1920, 1080};
	SceneMaterial mat {};

	// main loop
	float last_time { };
	bool quit = false;
	while (!quit) {
		SDL_Event e;
		ContextData ctx { Context::get()->data() };
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
					ctx.camera_pos.at(2) += 5;
					Context::get()->set(ctx);
					break;
				}
				case SDLK_A: {
					Vector3 cam_pos { ctx.camera_pos };
					ctx.camera_pos.at(0) -= 5;
					Context::get()->set(ctx);
					break;
				}
				case SDLK_S: {
					Vector3 cam_pos { ctx.camera_pos };
					ctx.camera_pos.at(2) -= 5;
					Context::get()->set(ctx);
					break;
				}
				case SDLK_D: {
					Vector3 cam_pos { ctx.camera_pos };
					ctx.camera_pos.at(0) += 5;
					Context::get()->set(ctx);
					break;
				}
				case SDLK_Z: {
					Vector3 cam_pos { ctx.camera_pos };
					ctx.camera_pos.at(1) += 5;
					Context::get()->set(ctx);
					break;
				}
				case SDLK_X: {
					Vector3 cam_pos { ctx.camera_pos };
					ctx.camera_pos.at(1) -= 5;
					Context::get()->set(ctx);
					break;
				}
				}
			}
		}
		mat.draw();

		// update time
		float new_time { SDL_GetTicks() / 1000.0f };
		ctx.delta_time = { new_time - last_time };
		Context::get()->set(ctx);
		last_time = new_time;
		renderer.update();
		SDL_Delay(10);
	}
	return 0;
}
