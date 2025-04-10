#include "Renderer.hpp"
#include "Context.hpp"

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
	SDL_Window *window = SDL_CreateWindow("3D", m_width, m_height, m_windowFlags);
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
		m_width, m_height,
		gpu,
		mutual_format,
		SDL_GetBasePath(),
		"shaders/source/",
		{30, 30, 30}
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
	SDL_Quit();
}

void Renderer::update() {
	ContextData ctx { Context::get()->data() };
	m_time = m_time + ctx.delta_time > SDL_PI_F * 2 ? 0.0f : m_time + ctx.delta_time;
	ctx.camera_pos = { SDL_cosf(m_time) * 30, 30, SDL_sinf(m_time) * 30 };
	Context::get()->set(ctx);
}
