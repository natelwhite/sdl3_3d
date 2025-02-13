#pragma once
#include <SDL3/SDL_gpu.h>

struct ContextData {
	public:
		SDL_Window *window;
		SDL_GPUDevice *gpu;
		SDL_GPUShaderFormat shader_format;
		const char *exe_path, *shaders_path;
};

class Context {
	public:
		Context(const Context &obj) = delete;
		static Context *get() {
			if (self == nullptr) {
				self = new Context();
			}
			return self;
		}
		void set(const ContextData &t_data) {
			this->m_data = t_data;
		}
		ContextData data() {
			return this->m_data;
		}
	private:
		ContextData m_data;

		static Context *self;
		Context() {}
		Context(ContextData t_data) : m_data(t_data) {}
};
