#pragma once
#include <SDL3_shadercross/SDL_shadercross.h>
#include <iostream>

#include "Buffer.hpp"
#include "Math.hpp"
#include "SDL3/SDL_gpu.h"

struct PositionColorVertex {
	float x, y, z;
	Uint8 r, g, b, a;
};

struct PositionVertex {
	float x, y, z;
};

struct PositionTextureVertex {
	float x, y, z;
	float u, v;
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
};

class SceneMaterial {
	public:
		SceneMaterial();
		~SceneMaterial();
		void draw();
		VertexBuffer<PositionColorVertex>* worldVertBuffer() { return &m_world_v; }
		IndexBuffer* worldIndexBuffer() { return &m_world_i; }
	private:
		void init();
		VertexBuffer<PositionColorVertex> m_world_v;
		IndexBuffer m_world_i;
		VertexBuffer<PositionTextureVertex> m_screen_v;
		IndexBuffer m_screen_i;
		SDL_GPUGraphicsPipeline *m_world_pipeline, *m_screen_pipeline;
		SDL_GPUTextureFormat m_swapchain_format;
		SDL_GPUGraphicsPipelineCreateInfo m_world_pipeline_create {
			.vertex_shader = nullptr, // loaded during .init()
			.fragment_shader = nullptr, // loaded during init()
			.vertex_input_state = (SDL_GPUVertexInputState){
				.vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[1]){ {
					.slot = 0,
					.pitch = sizeof(PositionColorVertex),
					.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
					.instance_step_rate = 0,
				}},
				.num_vertex_buffers = 1,
				.vertex_attributes = (SDL_GPUVertexAttribute[2]){ {
					.location = 0,
					.buffer_slot = 0,
					.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
					.offset = 0
				}, {
					.location = 1,
					.buffer_slot = 0,
					.format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,
					.offset = sizeof(float) * 3
				}},
				.num_vertex_attributes = 2,
			},
			.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			.rasterizer_state = (SDL_GPURasterizerState){
				.fill_mode = SDL_GPU_FILLMODE_FILL,
				.cull_mode = SDL_GPU_CULLMODE_NONE,
				.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE
			},
			.depth_stencil_state = (SDL_GPUDepthStencilState){
				.compare_op = SDL_GPU_COMPAREOP_LESS,
				.write_mask = 0xFF,
				.enable_depth_test = true,
				.enable_depth_write = true,
				.enable_stencil_test = false,
			},
			.target_info = {
				.color_target_descriptions = (SDL_GPUColorTargetDescription[1]){ {
					.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM
				}},
				.num_color_targets = 1,
				.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
				.has_depth_stencil_target = true
			}
		};
		SDL_GPUGraphicsPipelineCreateInfo m_screen_pipeline_create {
			.vertex_shader = nullptr, // loaded during .init()
			.fragment_shader = nullptr, // loaded during .init()
			.vertex_input_state = {
				.vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[1]){ {
					.slot = 0,
					.pitch = sizeof(PositionTextureVertex),
					.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
					.instance_step_rate = 0
				}},
				.num_vertex_buffers = 1,
				.vertex_attributes = (SDL_GPUVertexAttribute[2]){ {
					.location = 0,
					.buffer_slot = 0,
					.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
					.offset = 0
				}, {
					.location = 0,
					.buffer_slot = 0,
					.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
					.offset = sizeof(float) * 3
				}},
				.num_vertex_attributes = 2,
			},
			.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			.target_info = { // assigned during .init()
			},
		};
		SDL_GPUTexture *m_scene_color, *m_scene_depth;
		SDL_GPUTextureCreateInfo m_scene_color_create {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
			.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
			.width = 0, // assigned during .init()
			.height = 0, // assigned during .init()
			.layer_count_or_depth = 1,
			.num_levels = 1,
			.sample_count = SDL_GPU_SAMPLECOUNT_1
		};
		SDL_GPUTextureCreateInfo m_scene_depth_create {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
			.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
			.width = 0, // assigned during .init()
			.height = 0, // assigned during .init()
			.layer_count_or_depth = 1,
			.num_levels = 1,
			.sample_count = SDL_GPU_SAMPLECOUNT_1
		};
		SDL_GPUSampler *m_sampler;
		SDL_GPUSamplerCreateInfo m_sampler_create {
			.min_filter = SDL_GPU_FILTER_NEAREST,
			.mag_filter = SDL_GPU_FILTER_NEAREST,
			.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
			.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
			.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
			.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT
		};

		Matrix4x4 getFov(const float &fov, const float &aspect, const float &near, const float &far) const;
		Matrix4x4 getLookAt(const Vector3 &camera_pos, const Vector3 &camera_target, const Vector3 &camera_up) const;
		SDL_GPUShader* loadShader(const char *filename, const Uint32 &num_samplers, const Uint32 &num_uniform_buffers, const Uint32 &num_storage_buffers, const Uint32 &num_storage_textures);
};
