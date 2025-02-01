#include "Renderer.hpp"

int main() {
	Renderer renderer {640, 480};
	if (!renderer.init()) {
		return -1;
	}
	SDL_GPUShader *vert_shader = renderer.loadShader("RawTriangle.vert", 0, 0, 0, 0);
	if (vert_shader == nullptr) {
		return -1;
	}
	SDL_GPUShader *frag_shader = renderer.loadShader("SolidColor.frag", 0, 0, 0, 0);
	if (frag_shader == nullptr) {
		return -1;
	}

	SDL_GPUGraphicsPipeline *pipeline = renderer.createGraphicsPipeline(vert_shader, frag_shader);
	if (pipeline == nullptr) {
		return -1;
	}

	// no longer needed by gpu after making pipeline
	renderer.releaseShader(vert_shader);
	renderer.releaseShader(frag_shader);

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
					vert_shader = renderer.loadShader("RawTriangle.vert", 0, 0, 0, 0);
					frag_shader = renderer.loadShader("SolidColor.frag", 0, 0, 0, 0);
					if (vert_shader == nullptr) {
						SDL_Log("Could not reload vertex shader: %s", SDL_GetError());
						continue;
					}
					if (frag_shader == nullptr) {
						SDL_Log("Could not reload fragment shader: %s", SDL_GetError());
						continue;
					}
					SDL_GPUGraphicsPipeline *new_pipeline = renderer.createGraphicsPipeline(vert_shader, frag_shader);
					if (new_pipeline == nullptr) {
						SDL_Log("Could not create graphics pipeline: %s", SDL_GetError());
						continue;
					}

					renderer.releaseShader(vert_shader);
					renderer.releaseShader(frag_shader);
					renderer.releaseGraphicsPipeline(pipeline);
					pipeline = new_pipeline;
					break;
				}
			}
		}
		renderer.draw(pipeline);
	}
	renderer.releaseGraphicsPipeline(pipeline);
	renderer.quit();
	return 0;
}
