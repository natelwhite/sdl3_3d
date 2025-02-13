#include "Materials.hpp"

Material::Material(const char *t_vert_file, const char *t_frag_file)
	: m_vert_file(t_vert_file), m_frag_file(t_frag_file)  {
}

Material::~Material() {
	const ContextData ctx { Context::get()->data() };
	std::cout << "Release graphics pipeline" << std::endl;
	SDL_ReleaseGPUGraphicsPipeline(ctx.gpu, m_pipeline);
}

void Material::refresh() {
	const ContextData ctx { Context::get()->data() };
	std::cout << "Release graphics pipeline" << std::endl;
	SDL_ReleaseGPUGraphicsPipeline(ctx.gpu, m_pipeline);
	init();
}

SDL_GPUShader* Material::loadShader(const char *filename, Uint32 num_samplers, Uint32 num_uniform_buffers, Uint32 num_storage_buffers, Uint32 num_storage_textures) {
	// determine if vertex or fragment shader
	SDL_ShaderCross_ShaderStage shader_stage;
	if (SDL_strstr(filename, ".vert")) {
		shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
	} else if (SDL_strstr(filename, ".frag")) {
		shader_stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
	} else {
		SDL_Log("Invalid shader stage"); 
		return nullptr;
	}
	ContextData ctx { Context::get()->data() };
	// get the full path to the shader given it's name
	char full_path[256];
	SDL_snprintf(full_path, sizeof(full_path), "%s%s%s.hlsl", ctx.exe_path, ctx.shaders_path, filename);
	// get shader code
	const char *entrypoint = "main";
	size_t code_size;
	void *code = SDL_LoadFile(full_path, &code_size);
	if (code == NULL) {
		SDL_Log("Failed to load shader from disk: %s", full_path);
		return nullptr;
	}
	// create info & metadata for parsing hlsl to compile during runtime
	SDL_ShaderCross_HLSL_Info hlsl_info {
		.source = static_cast<const char*>(code),
		.entrypoint = entrypoint,
		.include_dir = NULL,
		.defines = NULL,
		.shader_stage = shader_stage,
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
	// compile hlsl to spv
	std::cout << "Create Shader" << std::endl;
	SDL_GPUShader *result = SDL_ShaderCross_CompileGraphicsShaderFromHLSL(ctx.gpu, &hlsl_info, &metadata);
	SDL_free(code);
	if (result == nullptr) {
		SDL_Log("Failed to create shader: %s", SDL_GetError());
		return nullptr;
	}
	return result;
}

BasicMat::BasicMat(const char *t_vert_file, const char *t_frag_file) 
	: Material(t_vert_file, t_frag_file) {
	init();
}

void BasicMat::init() {
	SDL_GPUShader *vert_shader = loadShader(m_vert_file, 0, 0, 0, 0);
	SDL_GPUShader *frag_shader = loadShader(m_frag_file, 0, 0, 0, 0);
	const SDL_GPUColorTargetBlendState target_blend_state {
		// color
		.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
		.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
		// alpha
		.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
		.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
	};
	ContextData ctx { Context::get()->data() };
	const SDL_GPUColorTargetDescription target_description {
		.format = SDL_GetGPUSwapchainTextureFormat(ctx.gpu, ctx.window),
		.blend_state = target_blend_state
	};
	const SDL_GPUGraphicsPipelineTargetInfo target_info {
		.color_target_descriptions = &target_description,
		.num_color_targets = 1,
	};
	const SDL_GPUGraphicsPipelineCreateInfo m_pipeline_info {
		.vertex_shader = vert_shader,
		.fragment_shader = frag_shader,
		/*.vertex_input_state = vert_input_state,*/
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.rasterizer_state = { SDL_GPU_FILLMODE_FILL },
		.target_info = target_info,
	};
	m_pipeline = SDL_CreateGPUGraphicsPipeline(ctx.gpu, &m_pipeline_info);
	if (m_pipeline == nullptr) {
		SDL_Log("CreateGPUGraphicsPipeline failed: %s", SDL_GetError());
	}
	std::cout << "Release shader" << std::endl;
	SDL_ReleaseGPUShader(ctx.gpu, vert_shader);
	std::cout << "Release shader" << std::endl;
	SDL_ReleaseGPUShader(ctx.gpu, frag_shader);
	vert_shader = nullptr;
	frag_shader = nullptr;
}

void BasicMat::draw() {
	const ContextData ctx { Context::get()->data() };
	SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(ctx.gpu);
	if (cmdbuf == nullptr) {
		SDL_Log("AcquireGPUCommandBuffer failed%s", SDL_GetError());
		return;
	}
	SDL_GPUTexture *swapchain_texture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, ctx.window, &swapchain_texture, NULL, NULL)) {
		SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
		return;
	}
	const SDL_FColor clear_color {0.0f, 0.0f, 0.0f, 1.0f};
	SDL_GPUColorTargetInfo target_info {
		.texture = swapchain_texture,
		.clear_color = clear_color,
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE
	};
	SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(cmdbuf, &target_info, 1, NULL);
	SDL_BindGPUGraphicsPipeline(render_pass, m_pipeline);
	SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);
	SDL_EndGPURenderPass(render_pass);
	SDL_SubmitGPUCommandBuffer(cmdbuf);
}

VertexBufferMat::VertexBufferMat(const char *t_vert_file, const char *t_frag_file, const size_t &t_vertex_count) 
	: Material(t_vert_file, t_frag_file), VertexBuffer<PositionColorVertex>(t_vertex_count), m_vertex_count(t_vertex_count) {
	init();
}

void VertexBufferMat::init() {
	SDL_GPUShader *vert_shader = loadShader(m_vert_file, 0, 0, 0, 0);
	SDL_GPUShader *frag_shader = loadShader(m_frag_file, 0, 0, 0, 0);
	const SDL_GPUColorTargetBlendState target_blend_state {
		// color
		.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
		.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
		// alpha
		.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
		.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
	};
	ContextData ctx { Context::get()->data() };
	const SDL_GPUColorTargetDescription target_description {
		.format = SDL_GetGPUSwapchainTextureFormat(ctx.gpu, ctx.window),
		.blend_state = target_blend_state
	};
	const SDL_GPUGraphicsPipelineTargetInfo target_info {
		.color_target_descriptions = &target_description,
		.num_color_targets = 1,
	};

	const SDL_GPUVertexBufferDescription vert_buff_description {
		.slot = 0,
		.pitch = sizeof(PositionColorVertex),
		.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
		.instance_step_rate = 0,
	};
	const SDL_GPUVertexAttribute vert_attributes[2] {
		{
			.location = 0,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = 0
		},
		{
			.location = 1,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,
			.offset = static_cast<Uint32>(sizeof(float) * 3)
		}
	};
	SDL_GPUVertexInputState vert_input_state {
		.vertex_buffer_descriptions = &vert_buff_description,
		.num_vertex_buffers = 1,
		.vertex_attributes = vert_attributes,
		.num_vertex_attributes = 2
	};
	const SDL_GPUGraphicsPipelineCreateInfo m_pipeline_info {
		.vertex_shader = vert_shader,
		.fragment_shader = frag_shader,
		.vertex_input_state = vert_input_state,
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.rasterizer_state = { SDL_GPU_FILLMODE_FILL },
		.target_info = target_info,
	};
	m_pipeline = SDL_CreateGPUGraphicsPipeline(ctx.gpu, &m_pipeline_info);
	if (m_pipeline == nullptr) {
		SDL_Log("CreateGPUGraphicsPipeline failed: %s", SDL_GetError());
	}
	std::cout << "Release shader" << std::endl;
	SDL_ReleaseGPUShader(ctx.gpu, vert_shader);
	std::cout << "Release shader" << std::endl;
	SDL_ReleaseGPUShader(ctx.gpu, frag_shader);
	vert_shader = nullptr;
	frag_shader = nullptr;
}

void VertexBufferMat::draw() {
	const ContextData ctx { Context::get()->data() };
	SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(ctx.gpu);
	if (cmdbuf == nullptr) {
		SDL_Log("AcquireGPUCommandBuffer failed%s", SDL_GetError());
		return;
	}
	SDL_GPUTexture *swapchain_texture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, ctx.window, &swapchain_texture, NULL, NULL)) {
		SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
		return;
	}
	const SDL_FColor clear_color {0.0f, 0.0f, 0.0f, 1.0f};
	SDL_GPUColorTargetInfo target_info {
		.texture = swapchain_texture,
		.clear_color = clear_color,
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE
	};
	SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(cmdbuf, &target_info, 1, NULL);
	SDL_BindGPUGraphicsPipeline(render_pass, m_pipeline);
	const SDL_GPUBufferBinding buffer_binding {
		.buffer = m_main_buffer,
		.offset = 0
	};
	SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);
	SDL_DrawGPUPrimitives(render_pass, m_vertex_count, 1, 0, 0);
	SDL_EndGPURenderPass(render_pass);
	SDL_SubmitGPUCommandBuffer(cmdbuf);
}

