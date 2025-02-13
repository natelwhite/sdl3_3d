#include "Renderer.hpp"
#include "Context.hpp"
#include "SDL3/SDL_gpu.h"

Renderer::Renderer(const int &t_width, const int &t_height) : m_width(t_width), m_height(t_height) {
	// init SDL video
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
		return;
	}
	// create gpu device
	SDL_GPUDevice *gpu = SDL_CreateGPUDevice(m_accepted_shader_formats, false, NULL);
	if (gpu == nullptr) {
		SDL_Log("CreateGPUDevice failed: %s", SDL_GetError());
		return;
	}
	SDL_GPUShaderFormat mutual_format { SDL_GetGPUShaderFormats(gpu) };
	// create window
	SDL_Window *window = SDL_CreateWindow("Hello World", m_width, m_height, m_windowFlags);
	if (window == nullptr) {
		SDL_Log("CreateWindow failed: %s", SDL_GetError());
		return;
	}
	// bind window to gpu
	if (!SDL_ClaimWindowForGPUDevice(gpu, window)) {
		SDL_Log("ClaimWindowForGPUDevice failed: %s", SDL_GetError());
		return;
	}
	const ContextData ctx {
		window,
		gpu,
		mutual_format,
		SDL_GetBasePath(),
		"shaders/source/"
	};
	Context::get()->set(ctx);
	return;
}

Renderer::~Renderer() {
	ContextData ctx { Context::get()->data() };
	SDL_ReleaseWindowFromGPUDevice(ctx.gpu, ctx.window);
	SDL_DestroyGPUDevice(ctx.gpu);
	SDL_DestroyWindow(ctx.window);
	Context::get()->set(ContextData{});
}

