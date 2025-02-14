#pragma once
#include <SDL3/SDL_gpu.h>
#include "Math.hpp"

struct ContextData {
	public:
		SDL_Window *window;
		Uint32 width, height;
		SDL_GPUDevice *gpu;
		SDL_GPUShaderFormat shader_format;
		const char *exe_path, *shaders_path;
		Vector3 camera_pos {0, 0, 4};
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

