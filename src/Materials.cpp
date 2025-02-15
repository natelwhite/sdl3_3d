#include "Materials.hpp"
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
#include "SDL3_shadercross/SDL_shadercross.h"

Material::Material(const char *t_vert_file, const char *t_frag_file)
	: m_vert_file(t_vert_file), m_frag_file(t_frag_file)  {
}

Material::~Material() {
	const ContextData ctx { Context::get()->data() };
	SDL_ReleaseGPUGraphicsPipeline(ctx.gpu, m_pipeline);
}

void Material::refresh() {
	const ContextData ctx { Context::get()->data() };
	SDL_ReleaseGPUGraphicsPipeline(ctx.gpu, m_pipeline);
	init();
}

Matrix4x4 SceneMaterial::getFov(const float &fov, const float &aspect, const float &near, const float &far)  const {
	const float num { 1.0f / static_cast<float>(SDL_tanf(fov * 0.5f)) };
	return Matrix4x4 {
		Vector4 { num / aspect, 0, 0, 0 },
		Vector4 { 0, num, 0, 0 },
		Vector4 { 0, 0, far / (near - far), -1 },
		Vector4 { 0, 0, (near * far) / (near - far), 0 },
	};
}

Matrix4x4 SceneMaterial::getLookAt(const Vector3 &camera_pos, const Vector3 &camera_target, const Vector3 &camera_up) const {
	const Vector3 target_to_pos {
		camera_pos.at(0) - camera_target.at(0),
		camera_pos.at(1) - camera_target.at(1),
		camera_pos.at(2) - camera_target.at(2),
	};
	const Vector3 a { target_to_pos.normalize() };
	const Vector3 b { camera_up.cross(a).normalize() };
	const Vector3 c { a.cross(b) };
	return {
		b.at(0), c.at(0), a.at(0), 0,
		b.at(1), c.at(1), a.at(1), 0,
		b.at(2), c.at(2), a.at(2), 0,
		-(b.dot(camera_pos), -(c.dot(camera_pos), -(a.dot(camera_pos))))
	};
}

SDL_GPUShader* SceneMaterial::loadShader(const char *filename, const Uint32 &num_samplers, const Uint32 &num_uniform_buffers, const Uint32 &num_storage_buffers, const Uint32 &num_storage_textures) {
	SDL_ShaderCross_ShaderStage stage;
	if (SDL_strstr(filename, ".vert")) {
		stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
	} else if (SDL_strstr(filename, ".frag")) {
		stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
	} else {
		SDL_Log("Invalid shader stage!");
		return nullptr;
	}
	const ContextData ctx { Context::get()->data() };
	char full_path[256];
	SDL_snprintf(full_path, sizeof(full_path), "%s%s%s%s", ctx.exe_path, ctx.shaders_path, filename, ".hlsl");
	size_t code_size;
	void *code { SDL_LoadFile(full_path, &code_size) };
	if (code == nullptr) {
		SDL_Log("LoadFile failed: %s", SDL_GetError());
		return nullptr;
	}
	SDL_ShaderCross_HLSL_Info shader_info {
		.source = static_cast<const char*>(code),
		.entrypoint = "main",
		.include_dir = NULL,
		.defines = NULL,
		.shader_stage = stage,
		.enable_debug = true,
		.name = NULL,
		.props = 0
	};
	SDL_ShaderCross_GraphicsShaderMetadata metadata {
		.num_samplers = num_samplers,
		.num_storage_textures = num_storage_textures,
		.num_storage_buffers = num_storage_buffers,
		.num_uniform_buffers = num_uniform_buffers
	};
	SDL_GPUShader *result { SDL_ShaderCross_CompileGraphicsShaderFromHLSL(ctx.gpu, &shader_info, &metadata) };
	if (result == nullptr) {
		SDL_Log("CompileGraphicsShaderFromHLSL failed: %s", SDL_GetError());
		return nullptr;
	}
	return result;
}

SceneMaterial::SceneMaterial()
	: m_world_v(24), m_world_i(36), m_screen_v(4), m_screen_i(6) {
	init();
}

void SceneMaterial::init() {
	// load shaders
	const char *shader_files[4] {
		"PositionColorTransform.vert",
		"SolidColorDepth.frag",
		"TexturedQuad.vert",
		"DepthOutline.frag"
	};
	SDL_GPUShader *shaders[4] {
		loadShader(shader_files[0], 0, 1, 0, 0),
		loadShader(shader_files[1], 0, 1, 0, 0),
		loadShader(shader_files[2], 0, 0, 0, 0),
		loadShader(shader_files[3], 2, 1, 0, 0)
	};
	const Uint8 WORLD_VERT { 0 }, WORLD_FRAG { 1 }, SCREEN_VERT { 2 }, SCREEN_FRAG { 3 };
	for (int i = 0; i < 4; ++i) {
		if (shaders[i] == nullptr) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadShader failed");
			return;
		}
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created GPUShader:\n\t%s", shader_files[i]);
	}
	// assign shaders to pipelines
	m_world_pipeline_create.vertex_shader = shaders[WORLD_VERT];
	m_world_pipeline_create.fragment_shader = shaders[WORLD_FRAG];
	m_screen_pipeline_create.vertex_shader = shaders[SCREEN_VERT];
	m_screen_pipeline_create.fragment_shader = shaders[SCREEN_FRAG];
	// assign info about swapchain texture
	const ContextData ctx { Context::get()->data() };
	const SDL_GPUColorTargetDescription swapchain_color_target_description[1] { {
		.format = SDL_GetGPUSwapchainTextureFormat(ctx.gpu, ctx.window),
		.blend_state = {
			.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
			.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.color_blend_op = SDL_GPU_BLENDOP_ADD,
			.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
			.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.alpha_blend_op = SDL_GPU_BLENDOP_ADD,
			.enable_blend = true
		}
	}};
	SDL_GPUGraphicsPipelineTargetInfo swapchain_target_info {
		.color_target_descriptions = swapchain_color_target_description,
		.num_color_targets = 1,
	};
	m_screen_pipeline_create.target_info = swapchain_target_info;
	// create world pipeline
	m_world_pipeline = SDL_CreateGPUGraphicsPipeline(ctx.gpu, &m_world_pipeline_create);
	if (m_world_pipeline == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateGPUGraphicsPipeline failed: %s", SDL_GetError());
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created GPUGraphicsPipeline");
	SDL_ReleaseGPUShader(ctx.gpu, shaders[WORLD_VERT]);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Released GPUShader:\n\t%s", shader_files[WORLD_VERT]);
	SDL_ReleaseGPUShader(ctx.gpu, shaders[WORLD_FRAG]);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Released GPUShader:\n\t%s", shader_files[WORLD_FRAG]);
	// create screen pipeline
	m_screen_pipeline = SDL_CreateGPUGraphicsPipeline(ctx.gpu, &m_screen_pipeline_create);
	if (m_screen_pipeline == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateGPUGraphicsPipeline failed: %s", SDL_GetError());
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created GPUGraphicsPipeline");
	SDL_ReleaseGPUShader(ctx.gpu, shaders[SCREEN_VERT]);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Released GPUShader:\n\t%s", shader_files[SCREEN_VERT]);
	SDL_ReleaseGPUShader(ctx.gpu, shaders[SCREEN_FRAG]);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Released GPUShader:\n\t%s", shader_files[SCREEN_FRAG]);
	// assign window dimensions to textures
	m_scene_color_create.width = ctx.width;
	m_scene_color_create.height = ctx.height;
	m_scene_depth_create.width = ctx.width;
	m_scene_depth_create.height = ctx.height;
	// create scene color & depth texture
	m_scene_color = SDL_CreateGPUTexture(ctx.gpu, &m_scene_color_create);
	if (m_scene_color == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateGPUTexture failed: %s", SDL_GetError());
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created GPUTexture");
	m_scene_depth = SDL_CreateGPUTexture(ctx.gpu, &m_scene_depth_create);
	if (m_scene_depth == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateGPUTexture failed: %s", SDL_GetError());
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created GPUTexture");
	// create sampler
	m_sampler = SDL_CreateGPUSampler(ctx.gpu, &m_sampler_create);
	if (m_sampler == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateGPUSampler failed: %s", SDL_GetError());
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created GPUSampler");
	const PositionColorVertex world_vertices[24] {
		{ -10, -10, -10, 255, 0, 0, 255 },
		{ 10, -10, -10, 255, 0, 0, 255 },
		{ 10, 10, -10, 255, 0, 0, 255 },
		{ -10, 10, -10, 255, 0, 0, 255 },

		{ -10, -10, 10, 255, 255, 0, 255 },
		{ 10, -10, 10, 255, 255, 0, 255 },
		{ 10, 10, 10, 255, 255, 0, 255 },
		{ -10, 10, 10, 255, 255, 0, 255 },

		{ -10, -10, -10, 255, 0, 255, 255 },
		{ -10, 10, -10, 255, 0, 255, 255 },
		{ -10, 10, 10, 255, 0, 255, 255 },
		{ -10, -10, 10, 255, 0, 255, 255 },

		{ 10, -10, -10, 0, 255, 0, 255 },
		{ 10, 10, -10, 0, 255, 0, 255 },
		{ 10, 10, 10, 0, 255, 0, 255 },
		{ 10, -10, 10, 0, 255, 0, 255 },

		{ -10, -10, -10, 255, 255, 255 },
		{ -10, -10, 10, 255, 255, 255 },
		{ 10, -10, 10, 255, 255, 255 },
		{ 10, -10, -10, 255, 255, 255 },

		{ -10, 10, -10, 0, 0, 255, 255 },
		{ -10, 10, 10, 0, 0, 255, 255 },
		{ 10, 10, 10, 0, 0, 255, 255 },
		{ 10, 10, -10, 0, 0, 255, 255 },
	};

	const Uint16 world_indices[36] {
		 0,  1,  2,  0,  2,  3,
		 6,  5,  4,  7,  6,  4,
		 8,  9, 10,  8, 10, 11,
		14, 13, 12, 15, 14, 12,
		16, 17, 18, 16, 18, 19,
		22, 21, 20, 23, 22, 20
	};
	// push verts & indices to buffer
	const PositionTextureVertex screen_vertices[4] {
		{-1, 1, 0, 0, 0},
		{1, 1, 0, 1, 0},
		{1, -1, 0, 1, 1},
		{-1, -1, 0, 0, 1}
	};
	const Uint16 screen_indices[6] { 0, 1, 2, 0, 1, 3 };
	SDL_memcpy(m_world_v.open(), world_vertices, sizeof(PositionColorVertex) * 24);
	m_world_v.upload();
	SDL_memcpy(m_world_i.open(), world_indices, sizeof(Uint16) * 36);
	m_world_i.upload();
	SDL_memcpy(m_screen_v.open(), screen_vertices, sizeof(PositionTextureVertex) * 4);
	m_screen_v.upload();
	SDL_memcpy(m_screen_i.open(), screen_indices, sizeof(Uint16) * 6);
	m_screen_i.upload();
}

SceneMaterial::~SceneMaterial() {
	const ContextData ctx { Context::get()->data() };
	SDL_ReleaseGPUGraphicsPipeline(ctx.gpu, m_world_pipeline);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Released GPUGraphicsPipeline");
	SDL_ReleaseGPUGraphicsPipeline(ctx.gpu, m_screen_pipeline);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Released GPUGraphicsPipeline");
	SDL_ReleaseGPUTexture(ctx.gpu, m_scene_color);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Released GPUTexture");
	SDL_ReleaseGPUTexture(ctx.gpu, m_scene_depth);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Released GPUTexture");
	SDL_ReleaseGPUSampler(ctx.gpu, m_sampler);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Released GPUSampler");
}

void SceneMaterial::draw() {
	ContextData ctx { Context::get()->data() };
	SDL_GPUCommandBuffer *cmdbuf { SDL_AcquireGPUCommandBuffer(ctx.gpu) };
	SDL_GPUTexture *swapchain;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, ctx.window, &swapchain, NULL, NULL)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
		return;
	}
	// do projection math
	float near_far[2] {20.0f, 60.0f};
	float aspect { static_cast<float>(ctx.width) / static_cast<float>(ctx.height) };
	Matrix4x4 proj { getFov(75.0f * SDL_PI_F / 180.0f, aspect, near_far[0], near_far[1]) };
	Matrix4x4 view { getLookAt(ctx.camera_pos, {0, 0, 0}, {0, 1, 0}) };
	Matrix4x4 view_proj { view * proj };
	float view_proj_flat[16] { };
	int i { };
	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {
			view_proj_flat[i] = view_proj.at(r).at(c);
			++i;
		}
	}
	SDL_PushGPUVertexUniformData(cmdbuf, 0, &view_proj_flat, sizeof(view_proj_flat));
	SDL_PushGPUFragmentUniformData(cmdbuf, 0, near_far, sizeof(near_far));
	// setup target info
	const SDL_GPUColorTargetInfo world_color_target_info {
		.texture = m_scene_color,
		.clear_color = {0, 0, 0, 0},
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE
	};
	const SDL_GPUDepthStencilTargetInfo depth_stencil_target_info {
		.texture = m_scene_depth,
		.clear_depth = 1,
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE,
		.cycle = true,
		.clear_stencil = 0
	};
	// render to screen texture
	SDL_GPURenderPass *render_pass { SDL_BeginGPURenderPass(cmdbuf, &world_color_target_info, 1, &depth_stencil_target_info)};
	const SDL_GPUBufferBinding world_buffer_binding_v { m_world_v.get(), 0 };
	const SDL_GPUBufferBinding world_buffer_binding_i { m_world_i.get(), 0 };
	SDL_BindGPUVertexBuffers(render_pass, 0, &world_buffer_binding_v, 1);
	SDL_BindGPUIndexBuffer(render_pass, &world_buffer_binding_i, SDL_GPU_INDEXELEMENTSIZE_16BIT);
	SDL_BindGPUGraphicsPipeline(render_pass, m_world_pipeline);
	SDL_DrawGPUIndexedPrimitives(render_pass, m_world_i.getCount(), 1, 0, 0, 0);
	SDL_EndGPURenderPass(render_pass);
	// render post processing
	const SDL_GPUColorTargetInfo screen_color_target_info {
		.texture = swapchain,
		.clear_color = {0.2, 0.5, 0.4, 1.0},
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE
	};
	render_pass = SDL_BeginGPURenderPass(cmdbuf, &screen_color_target_info, 1, NULL);
	SDL_BindGPUGraphicsPipeline(render_pass, m_screen_pipeline);
	const SDL_GPUBufferBinding screen_buffer_binding_v { m_screen_v.get(), 0 };
	const SDL_GPUBufferBinding screen_buffer_binding_i { m_screen_i.get(), 0 };
	SDL_BindGPUVertexBuffers(render_pass, 0, &screen_buffer_binding_v, 1);
	SDL_BindGPUIndexBuffer(render_pass, &screen_buffer_binding_i, SDL_GPU_INDEXELEMENTSIZE_16BIT);
	SDL_BindGPUGraphicsPipeline(render_pass, m_screen_pipeline);
	const SDL_GPUTextureSamplerBinding texture_sampler_bindings[2] {
		{m_scene_color, m_sampler},
		{m_scene_depth, m_sampler}
	};
	SDL_BindGPUFragmentSamplers(render_pass, 0, texture_sampler_bindings, 2);
	SDL_DrawGPUIndexedPrimitives(render_pass, m_screen_i.getCount(), 1, 0, 0, 0);
	SDL_EndGPURenderPass(render_pass);
	SDL_SubmitGPUCommandBuffer(cmdbuf);
}
