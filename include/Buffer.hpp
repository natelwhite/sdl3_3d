#pragma once
#include <SDL3/SDL_log.h>
#include "Context.hpp"
#include "SDL3/SDL_gpu.h"
#include <iostream>

template<typename STORAGE_TYPE> class Buffer {
	public:
		Buffer(const size_t &t_count, const SDL_GPUTextureUsageFlags &buffer_usage) : m_count(t_count) {
			const ContextData ctx { Context::get()->data() };
			const SDL_GPUBufferCreateInfo main_buff_info {
				.usage = buffer_usage,
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
		~Buffer() {
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
			const SDL_GPUTransferBufferLocation transfer_buffer_loc {
				.transfer_buffer = m_transfer_buffer,
				.offset = 0
			};
			const SDL_GPUBufferRegion buffer_region {
				.buffer = m_main_buffer,
				.offset = 0,
				.size = static_cast<Uint32>(sizeof(STORAGE_TYPE) * m_count)
			};
			SDL_UploadToGPUBuffer(copy_pass, &transfer_buffer_loc, &buffer_region, false);
			SDL_EndGPUCopyPass(copy_pass);
			SDL_SubmitGPUCommandBuffer(upload_cmd_buf);
		}
	SDL_GPUBuffer* get() const { return m_main_buffer; }
	size_t getCount() const { return m_count; }
	private:
		SDL_GPUBuffer *m_main_buffer { nullptr };
		const size_t m_count;
		SDL_GPUTransferBuffer *m_transfer_buffer { nullptr };
};

template<typename STORAGE_TYPE> class VertexBuffer : public Buffer<STORAGE_TYPE> {
	public:
		VertexBuffer(const size_t &t_count) : Buffer<STORAGE_TYPE>(t_count, SDL_GPU_BUFFERUSAGE_VERTEX) { }
};

template<typename STORAGE_TYPE> class IndexBuffer : public Buffer<STORAGE_TYPE> {
	public:
		IndexBuffer(const size_t &t_count) : Buffer<STORAGE_TYPE>(t_count, SDL_GPU_BUFFERUSAGE_INDEX) { }
};

