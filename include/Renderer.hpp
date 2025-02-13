#pragma once
#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>

typedef struct PositionColorVertex {
	float x, y, z;
	Uint8 r, g, b, a;
} PositionColorVertex;

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

template<typename STORAGE_TYPE> class VertexBuffer {
	public:
		VertexBuffer(const size_t &t_count) : m_count(t_count) {
			const ContextData ctx { Context::get()->data() };
			const SDL_GPUBufferCreateInfo main_buff_info {
				.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
				.size = static_cast<Uint32>(sizeof(STORAGE_TYPE) * m_count)
			};
			m_main_buffer = SDL_CreateGPUBuffer(ctx.gpu, &main_buff_info);
			if (m_main_buffer == nullptr) {
				SDL_Log("CreateGPUBuffer failed: %s", SDL_GetError());
				return;
			}
			const SDL_GPUTransferBufferCreateInfo trans_buff_info {
				.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
				.size = static_cast<Uint32>(sizeof(STORAGE_TYPE) * m_count)
			};
			m_transfer_buffer = SDL_CreateGPUTransferBuffer(ctx.gpu, &trans_buff_info);
			if (m_transfer_buffer == nullptr) {
				SDL_Log("CreateGPUTransferBuffer failed: %s", SDL_GetError());
				return;
			}
		}
		~VertexBuffer() {
			const ContextData ctx { Context::get()->data() };
			SDL_ReleaseGPUTransferBuffer(ctx.gpu, m_transfer_buffer);
			SDL_ReleaseGPUBuffer(ctx.gpu, m_main_buffer);
			m_main_buffer = nullptr;
			m_transfer_buffer = nullptr;
		}
		STORAGE_TYPE* open() {
			const ContextData ctx { Context::get()->data() };
			STORAGE_TYPE *transfer_data = static_cast<STORAGE_TYPE*>(SDL_MapGPUTransferBuffer(ctx.gpu, m_transfer_buffer, false));
			if (transfer_data == nullptr) {
				SDL_Log("MapGPUTransferBuffer failed: %s", SDL_GetError());
				return nullptr;
			}
			return transfer_data;
		}
		void upload() {
			const ContextData ctx { Context::get()->data() };
			SDL_UnmapGPUTransferBuffer(ctx.gpu, m_transfer_buffer);
			SDL_GPUCommandBuffer *upload_cmd_buf { SDL_AcquireGPUCommandBuffer(ctx.gpu) };
			SDL_GPUCopyPass *copy_pass { SDL_BeginGPUCopyPass(upload_cmd_buf) };
			SDL_GPUTransferBufferLocation transfer_buffer_loc {
				.transfer_buffer = m_transfer_buffer,
				.offset = 0
			};
			SDL_GPUBufferRegion buffer_region {
				.buffer = m_main_buffer,
				.offset = 0,
				.size = static_cast<Uint32>(sizeof(STORAGE_TYPE) * m_count)
			};
			SDL_UploadToGPUBuffer(copy_pass, &transfer_buffer_loc, &buffer_region, false);
			SDL_EndGPUCopyPass(copy_pass);
			SDL_SubmitGPUCommandBuffer(upload_cmd_buf);
		}
		SDL_GPUBuffer* address() { return m_main_buffer; }
	private:
		const size_t m_count;
		SDL_GPUBuffer *m_main_buffer { nullptr };
		SDL_GPUTransferBuffer *m_transfer_buffer { nullptr };
};

class Material {
	public:
		Material(const char *t_vert_file, const char *t_frag_file);
		~Material();
		virtual void draw() = 0;
		void refresh();
	protected:
		virtual void init() = 0;
		const char *m_vert_file, *m_frag_file;
		SDL_GPUGraphicsPipeline *m_pipeline;
		SDL_GPUGraphicsPipelineCreateInfo m_pipeline_info;

		SDL_GPUShader* loadShader(const char *filename, Uint32 num_samplers, Uint32 num_uniform_buffers, Uint32 num_storag_buffers, Uint32 num_storage_textures);
};

class BasicMat : public Material {
	public:
		BasicMat(const char *t_vert_file, const char *t_frag_file);
		void draw();
	private:
		void init();
};

class VertexBufferMat : public Material {
	public:
		VertexBufferMat(const char *t_vert_file, const char *t_frag_file, const size_t &t_vertex_count);
		void draw();
		VertexBuffer<PositionColorVertex> getBuffer() { return m_buffer; }
	private:
		void init();
		VertexBuffer<PositionColorVertex> m_buffer;
		const size_t m_vertex_count;
};

class Renderer {
	public:
		Renderer(const int &t_width, const int &t_height);
		SDL_GPUGraphicsPipeline* createGraphicsPipeline(SDL_GPUShader *vert_shader, SDL_GPUShader *frag_shader, SDL_GPUVertexInputState vert_input_state);

	private:
		Uint32 m_width, m_height; // window width & height
		const SDL_WindowFlags m_windowFlags = SDL_WINDOW_RESIZABLE;
		const SDL_GPUShaderFormat m_accepted_shader_formats = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;
};

