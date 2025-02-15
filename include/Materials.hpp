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
		SceneMaterial(const size_t &world_vert_count, const size_t &world_index_count);
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
		SDL_GPUTexture *m_scene_color, *m_scene_depth;
		SDL_GPUSampler *m_sampler;

		Matrix4x4 getFov(const float &fov, const float &aspect, const float &near, const float &far) const;
		Matrix4x4 getLookAt(const Vector3 &camera_pos, const Vector3 &camera_target, const Vector3 &camera_up) const;
		SDL_GPUShader* loadShader(const char *filename, const Uint32 &num_samplers, const Uint32 &num_uniform_buffers, const Uint32 &num_storage_buffers, const Uint32 &num_storage_textures);
};
