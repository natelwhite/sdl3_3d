#pragma once
#include <SDL3_shadercross/SDL_shadercross.h>
#include <iostream>

#include "Buffer.hpp"
#include "Math.hpp"

struct PositionColorVertex {
	float x, y, z;
	Uint8 r, g, b, a;
};

struct PositionVertex {
	float x, y, z;
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

		Matrix4x4 getFov(const float &fov, const float &aspect, const float &near, const float &far) const;
		Matrix4x4 getLookAt(const Vector3 &camera_pos, const Vector3 &camera_target, const Vector3 &camera_up) const;
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
		VertexBuffer<PositionColorVertex>* vertBuffer() { return &m_buffer; }
	private:
		void init();
		VertexBuffer<PositionColorVertex> m_buffer;
};

class ThreeDMat : public Material {
	public:
		ThreeDMat(const char *t_vert_file, const char *t_frag_file, const size_t &t_vertex_count, const size_t &t_index_count);
		~ThreeDMat();
		void draw();
		VertexBuffer<PositionVertex>* vertBuffer() { return &m_vert_buffer; }
		IndexBuffer<Uint16>* indexBuffer() { return &m_index_buffer; }
	private:
		void init();
		VertexBuffer<PositionVertex> m_vert_buffer;
		IndexBuffer<Uint16> m_index_buffer;
		SDL_GPUTexture *m_texture;
		SDL_GPUSampler *m_sampler;
};
