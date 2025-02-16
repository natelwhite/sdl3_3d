#pragma once
#include <SDL3/SDL.h>

class Renderer {
	public:
		Renderer(const int &t_width, const int &t_height);
		~Renderer();
		void update();

	private:
		float m_time { };
		Uint32 m_width, m_height; // window width & height
		const SDL_WindowFlags m_windowFlags = SDL_WINDOW_ALWAYS_ON_TOP;
		const SDL_GPUShaderFormat m_accepted_shader_formats = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;
};

