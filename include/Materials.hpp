#pragma once
#include <SDL3_shadercross/SDL_shadercross.h>

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

Matrix4x4 CreateProjection(const float &fov, const float &aspect, const float &near, const float &far);
Matrix4x4 CreateView(const Vector3 &camera_pos, const Vector3 &camera_target, const Vector3 &camera_up);
SDL_GPUShader* LoadShader(const ContextData &ctx, const char *filename, const Uint32 &num_samplers, const Uint32 &num_uniform_buffers, const Uint32 &num_storage_buffers, const Uint32 &num_storage_textures);

class SceneMaterial {
	public:
		SceneMaterial();
		~SceneMaterial();
		void draw();
		VertexBuffer<PositionColorVertex>* worldVertBuffer() { return &m_world_v; }
		IndexBuffer* worldIndexBuffer() { return &m_world_i; }
	private:
		int init();
		bool loadShaders(const ContextData &ctx);
		bool createWorldPipeline(const ContextData &ctx);
		bool createScreenPipeline(const ContextData &ctx);
		bool createColorTexture(const ContextData &ctx);
		bool createDepthTexture(const ContextData &ctx);
		bool createSampler(const ContextData &ctx);
		float m_time {};
		std::array<SDL_GPUShader*, 4> m_shaders;
		VertexBuffer<PositionColorVertex> m_world_v;
		IndexBuffer m_world_i;
		VertexBuffer<PositionTextureVertex> m_screen_v;
		IndexBuffer m_screen_i;
		SDL_GPUGraphicsPipeline *m_world_pipeline, *m_screen_pipeline;
		SDL_GPUTexture *m_scene_color, *m_scene_depth;
		SDL_GPUSampler *m_sampler;
};
