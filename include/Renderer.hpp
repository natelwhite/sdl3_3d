#pragma once
#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>

class Renderer {
	public:
		Renderer(const int &t_width, const int &t_height);
		~Renderer();
		SDL_GPUGraphicsPipeline* createGraphicsPipeline(SDL_GPUShader *vert_shader, SDL_GPUShader *frag_shader, SDL_GPUVertexInputState vert_input_state);

	private:
		Uint32 m_width, m_height; // window width & height
		const SDL_WindowFlags m_windowFlags = SDL_WINDOW_RESIZABLE;
		const SDL_GPUShaderFormat m_accepted_shader_formats = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;
};

