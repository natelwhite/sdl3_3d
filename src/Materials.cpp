#include "Materials.hpp"
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
#include "SDL3_shadercross/SDL_shadercross.h"

SceneMaterial::SceneMaterial()
	: m_world_v(24), m_world_i(36), m_screen_v(4), m_screen_i(6) {
	init();
}

bool SceneMaterial::loadShaders(const ContextData &ctx) {
	const char *shader_files[4] {
		"PositionColorTransform.vert",
		"SolidColorDepth.frag",
		"TexturedQuad.vert",
		"DepthOutline.frag"
	};
	m_shaders.at(0) = LoadShader(ctx, shader_files[0], 0, 1, 0, 0);
	m_shaders.at(1) = LoadShader(ctx, shader_files[1], 0, 1, 0, 0);
	m_shaders.at(2) = LoadShader(ctx, shader_files[2], 0, 0, 0, 0);
	m_shaders.at(3) = LoadShader(ctx, shader_files[3], 2, 1, 0, 0);
	for (int i = 0; i < 4; ++i) {
		if (m_shaders.at(i) == nullptr) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadShader failed");
			return false;
		}
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created GPUShader:\n\t%s", shader_files[i]);
	}
	return true;
}
bool SceneMaterial::createWorldPipeline(const ContextData &ctx) {
	const SDL_GPUVertexBufferDescription vertex_buffer_descriptions[1] {
		{
			.slot = 0,
			.pitch = sizeof(PositionColorVertex),
			.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
			.instance_step_rate = 0,
		}
	};
	const SDL_GPUVertexAttribute vertex_attributes[2] {
		{
			.location = 0,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = 0
		}, {
			.location = 1,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,
			.offset = sizeof(float) * 3
		}
	};
	const SDL_GPUColorTargetDescription color_target_descriptions[1] {
		{ .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM }
	};
	const SDL_GPUGraphicsPipelineCreateInfo world_pipeline_create {
		.vertex_shader = m_shaders.at(0),
		.fragment_shader = m_shaders.at(1),
		.vertex_input_state = {
			.vertex_buffer_descriptions = vertex_buffer_descriptions,
			.num_vertex_buffers = 1,
			.vertex_attributes = vertex_attributes,
			.num_vertex_attributes = 2,
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.rasterizer_state = {
			.fill_mode = SDL_GPU_FILLMODE_FILL,
			.cull_mode = SDL_GPU_CULLMODE_NONE,
			.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE
		},
		.depth_stencil_state = {
			.compare_op = SDL_GPU_COMPAREOP_LESS,
			.write_mask = 0xFF,
			.enable_depth_test = true,
			.enable_depth_write = true,
			.enable_stencil_test = false,
		},
		.target_info = {
			.color_target_descriptions = color_target_descriptions,
			.num_color_targets = 1,
			.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
			.has_depth_stencil_target = true
		}
	};
	m_world_pipeline = SDL_CreateGPUGraphicsPipeline(ctx.gpu, &world_pipeline_create);
	if (m_world_pipeline == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateGPUGraphicsPipeline failed: %s", SDL_GetError());
		return false;
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created GPUGraphicsPipeline");
	SDL_ReleaseGPUShader(ctx.gpu, m_shaders.at(0));
	SDL_ReleaseGPUShader(ctx.gpu, m_shaders.at(1));
	return true;
}

bool SceneMaterial::createScreenPipeline(const ContextData &ctx) {
	const SDL_GPUVertexBufferDescription vertex_buffer_descriptions[1] {
		{
			.slot = 0,
			.pitch = sizeof(PositionTextureVertex),
			.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
			.instance_step_rate = 0
		}
	};
	const SDL_GPUVertexAttribute vertex_attributes[2] {
		{
			.location = 0,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = 0
		}, {
			.location = 1,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
			.offset = sizeof(float) * 3
		}
	};
	const SDL_GPUColorTargetDescription color_target_description[1] { {
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
	const SDL_GPUGraphicsPipelineCreateInfo screen_pipeline_create {
		.vertex_shader = m_shaders.at(2), // assigned during .init()
		.fragment_shader = m_shaders.at(3), // assigned during .init()
		.vertex_input_state = {
			.vertex_buffer_descriptions = vertex_buffer_descriptions,
			.num_vertex_buffers = 1,
			.vertex_attributes = vertex_attributes,
			.num_vertex_attributes = 2,
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.target_info = {
			.color_target_descriptions = color_target_description,
			.num_color_targets = 1,
		},
	};
	m_screen_pipeline = SDL_CreateGPUGraphicsPipeline(ctx.gpu, &screen_pipeline_create);
	if (m_screen_pipeline == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateGPUGraphicsPipeline failed: %s", SDL_GetError());
		return false;
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created GPUGraphicsPipeline");
	SDL_ReleaseGPUShader(ctx.gpu, m_shaders.at(2));
	SDL_ReleaseGPUShader(ctx.gpu, m_shaders.at(3));
	return true;
}

bool SceneMaterial::createColorTexture(const ContextData &ctx) {
	const SDL_GPUTextureCreateInfo scene_color_create {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
		.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
		.width = ctx.width,
		.height = ctx.height,
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.sample_count = SDL_GPU_SAMPLECOUNT_1
	};
	m_scene_color = SDL_CreateGPUTexture(ctx.gpu, &scene_color_create);
	if (m_scene_color == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateGPUTexture failed: %s", SDL_GetError());
		return false;
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created GPUTexture");
	return true;
}

bool SceneMaterial::createDepthTexture(const ContextData &ctx) {
	const SDL_GPUTextureCreateInfo scene_depth_create {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
		.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
		.width = ctx.width,
		.height = ctx.height,
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.sample_count = SDL_GPU_SAMPLECOUNT_1
	};
	m_scene_depth = SDL_CreateGPUTexture(ctx.gpu, &scene_depth_create);
	if (m_scene_depth == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateGPUTexture failed: %s", SDL_GetError());
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created GPUTexture");
	return true;
}

bool SceneMaterial::createSampler(const ContextData &ctx) {
	const SDL_GPUSamplerCreateInfo sampler_create {
		.min_filter = SDL_GPU_FILTER_NEAREST,
		.mag_filter = SDL_GPU_FILTER_NEAREST,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT
	};
	m_sampler = SDL_CreateGPUSampler(ctx.gpu, &sampler_create);
	if (m_sampler == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateGPUSampler failed: %s", SDL_GetError());
		return false;
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created GPUSampler");
	return true;
}

void SceneMaterial::init() {
	// load shaders
	const ContextData ctx { Context::get()->data() };
	loadShaders(ctx);
	// create world pipeline
	createWorldPipeline(ctx);
	// create screen pipeline
	createScreenPipeline(ctx);
	// create textures
	createColorTexture(ctx);
	createDepthTexture(ctx);
	// create sampler
	createSampler(ctx);
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
	const Uint16 screen_indices[6] { 0, 1, 2, 0, 2, 3 };
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
	float near_far[2] {0.01f, 100.0f};
	float aspect { static_cast<float>(ctx.width) / static_cast<float>(ctx.height) };
	Matrix4x4 proj { CreateProjection(75.0f * SDL_PI_F / 180.0f, aspect, near_far[0], near_far[1]) };
	Matrix4x4 view { CreateView(ctx.camera_pos, {0, 0, 0}, {0, 1, 0}) };
	Matrix4x4 view_proj { view * proj };
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
		.stencil_load_op = SDL_GPU_LOADOP_CLEAR,
		.stencil_store_op = SDL_GPU_STOREOP_STORE,
		.cycle = true,
		.clear_stencil = 0,
	};
	SDL_PushGPUVertexUniformData(cmdbuf, 0, &view_proj, sizeof(view_proj));
	SDL_PushGPUFragmentUniformData(cmdbuf, 0, near_far, sizeof(near_far));
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

Matrix4x4 CreateProjection(const float &fov, const float &aspect, const float &near, const float &far) {
	const float num { 1.0f / static_cast<float>(SDL_tanf(fov * 0.5f)) };
	return Matrix4x4 {
		Vector4 { num / aspect, 0, 0, 0 },
		Vector4 { 0, num, 0, 0 },
		Vector4 { 0, 0, far / (near - far), -1 },
		Vector4 { 0, 0, (near * far) / (near - far), 0 },
	};
}

Matrix4x4 CreateView(const Vector3 &camera_pos, const Vector3 &camera_target, const Vector3 &camera_up) {
	const Vector3 target_to_pos {
		camera_pos.at(0) - camera_target.at(0),
		camera_pos.at(1) - camera_target.at(1),
		camera_pos.at(2) - camera_target.at(2),
	};
	const Vector3 a { target_to_pos.normalize() };
	const Vector3 b { camera_up.cross(a).normalize() };
	const Vector3 c { a.cross(b) };
	Matrix4x4 result {
		Vector4 { b.at(0), c.at(0), a.at(0), 0 },
		Vector4 { b.at(1), c.at(1), a.at(1), 0 },
		Vector4 { b.at(2), c.at(2), a.at(2), 0 },
		Vector4 { -(b.dot(camera_pos)), -(c.dot(camera_pos)), -(a.dot(camera_pos)), 1 }
	};
	return result;
}

SDL_GPUShader* LoadShader(const ContextData &ctx, const char *filename, const Uint32 &num_samplers, const Uint32 &num_uniform_buffers, const Uint32 &num_storage_buffers, const Uint32 &num_storage_textures) {
	SDL_ShaderCross_ShaderStage stage;
	if (SDL_strstr(filename, ".vert")) {
		stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
	} else if (SDL_strstr(filename, ".frag")) {
		stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
	} else {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid shader stage!");
		return nullptr;
	}
	char full_path[256];
	SDL_snprintf(full_path, sizeof(full_path), "%s%s%s%s", ctx.exe_path, ctx.shaders_path, filename, ".hlsl");
	size_t code_size;
	void *code { SDL_LoadFile(full_path, &code_size) };
	if (code == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LoadFile failed: %s", SDL_GetError());
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
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CompileGraphicsShaderFromHLSL failed: %s", SDL_GetError());
		return nullptr;
	}
	return result;
}
