#pragma once
#include "Buffer.hpp"
#include <SDL3_shadercross/SDL_shadercross.h>
#include <iostream>

typedef struct PositionColorVertex {
	float x, y, z;
	Uint8 r, g, b, a;
} PositionColorVertex;

struct Vector3 : std::array<float, 3> {
	Vector3 normalize() const;
	float dot(const Vector3 &other) const;
	Vector3 cross(const Vector3 &other) const;
};
struct Vector4 : std::array<float, 4> {};
struct Matrix4x4 : std::array<Vector4, 4> {
	Matrix4x4 operator * (const Matrix4x4 &other);
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

		const Matrix4x4 getFov(const float &fov, const float &aspect, const float &near, const float &far);
		const Matrix4x4 getLookAt(const Vector3 &camera_pos, const Vector3 &camera_target, const Vector3 &camera_up);
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
