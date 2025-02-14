#pragma once
#include "VertexBuffer.hpp"
#include <SDL3_shadercross/SDL_shadercross.h>
#include <iostream>

typedef struct PositionColorVertex {
	float x, y, z;
	Uint8 r, g, b, a;
} PositionColorVertex;

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

class VertexBufferMat : public Material, public VertexBuffer<PositionColorVertex> {
	public:
		VertexBufferMat(const char *t_vert_file, const char *t_frag_file, const size_t &t_vertex_count);
		void draw();
	private:
		void init();
		const size_t m_vertex_count;
};
