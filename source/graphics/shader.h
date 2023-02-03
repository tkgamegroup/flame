#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct DescriptorPool
		{
			virtual ~DescriptorPool() {}

			struct Current
			{
				virtual DescriptorPoolPtr operator()() = 0;
			};
			FLAME_GRAPHICS_API static Current& current;

			struct Create
			{
				virtual DescriptorPoolPtr operator()() = 0;
			};
			FLAME_GRAPHICS_API static Create& create;
		};

		struct DescriptorBinding
		{
			DescriptorType type = DescriptorNone;
			uint count = 1;
			std::string name;

			UdtInfo* ui = nullptr;
		};

		struct DescriptorSetLayout
		{
			std::vector<DescriptorBinding> bindings;
			std::unordered_map<uint, uint> bindings_map;

			std::filesystem::path filename;
			uint ref = 0;

			virtual ~DescriptorSetLayout() {}

			inline int find_binding(uint hash) const
			{
				auto it = bindings_map.find(hash);
				if (it != bindings_map.end())
					return it->second;
				return -1;
			}

			inline const DescriptorBinding& get_binding(uint hash) const
			{
				auto idx = find_binding(hash);
				if (idx != -1)
					return bindings[idx];
				static DescriptorBinding empty;
				return empty;
			}

			UdtInfo* get_buf_ui(uint hash) const
			{
				auto idx = find_binding(hash);
				if (idx != -1)
				{
					auto& binding = bindings[idx];
					assert(binding.type == DescriptorUniformBuffer ||
						binding.type == DescriptorStorageBuffer);
					return binding.ui;
				}
				return nullptr;
			}

			struct Create
			{
				virtual DescriptorSetLayoutPtr operator()(std::span<DescriptorBinding> bindings) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;

			struct Get
			{
				virtual DescriptorSetLayoutPtr operator()(const std::filesystem::path& filename) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;

			struct Release
			{
				virtual void operator()(DescriptorSetLayoutPtr layout) = 0;
			};
			FLAME_GRAPHICS_API static Release& release;
		};

		struct DescriptorSet
		{
			DescriptorSetLayoutPtr layout;

			virtual ~DescriptorSet() {}

			virtual void set_buffer_i(uint binding, uint index, BufferPtr buf, uint offset = 0, uint range = 0) = 0;
			inline void set_buffer(uint hash, uint index, BufferPtr buf, uint offset = 0, uint range = 0)
			{
				auto dsl = (DescriptorSetLayout*)layout;
				auto idx = dsl->find_binding(hash);
				if (idx == -1)
				{
					printf("descriptor set bind buffer failed: cannot find %d\n", hash);
					return;
				}
				auto& bd = dsl->bindings[idx];
				if (!is_one_of(bd.type, { DescriptorUniformBuffer, DescriptorStorageBuffer }))
				{
					printf("descriptor set bind buffer failed: type mismatch with %s\n", bd.name.c_str());
					return;
				}
				set_buffer_i(idx, index, buf, offset, range);
			}
			virtual void set_image_i(uint binding, uint index, ImageViewPtr iv, SamplerPtr sp) = 0;
			inline void set_image(uint hash, uint index, ImageViewPtr iv, SamplerPtr sp)
			{
				auto dsl = (DescriptorSetLayout*)layout;
				auto idx = dsl->find_binding(hash);
				if (idx == -1)
				{
					printf("descriptor set bind image failed: cannot find %d\n", hash);
					return;
				}
				auto& bd = dsl->bindings[idx];
				if (!is_one_of(bd.type, { DescriptorSampledImage, DescriptorStorageImage }))
				{
					printf("descriptor set bind image failed: type mismatch with %s\n", bd.name.c_str());
					return;
				}
				set_image_i(idx, index, iv, sp);
			}

			virtual void update() = 0;

			struct Create
			{
				virtual DescriptorSetPtr operator()(DescriptorPoolPtr pool, DescriptorSetLayoutPtr layout) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;
		};

		struct PipelineLayout
		{
			std::vector<DescriptorSetLayoutPtr> dsls;

			UdtInfo* pc_ui = nullptr;
			uint pc_sz = 0;

			std::filesystem::path filename;
			uint ref = 0;

			virtual ~PipelineLayout() {}

			struct Create
			{
				virtual PipelineLayoutPtr operator()(std::span<DescriptorSetLayoutPtr> dsls, uint push_constant_size) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;

			struct Get
			{
				virtual PipelineLayoutPtr operator()(const std::filesystem::path& filename) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;

			struct Release
			{
				virtual void operator()(PipelineLayoutPtr layout) = 0;
			};
			FLAME_GRAPHICS_API static Release& release;
		};

		struct Shader
		{
			ShaderStageFlags type = ShaderStageNone;
			std::filesystem::path filename;
			std::vector<std::string> defines;
			uint ref = 0;

			UdtInfo* in_ui = nullptr;
			UdtInfo* out_ui = nullptr;

			std::vector<std::pair<uint, void*>> dependencies;

			virtual ~Shader() {}

			virtual void recreate() = 0;

			struct Create
			{
				virtual ShaderPtr operator()(ShaderStageFlags type, const std::string& content, const std::vector<std::string>& defines, 
					const std::filesystem::path& dst = L"", const std::filesystem::path& src = L"") = 0;
			};
			FLAME_GRAPHICS_API static Create& create;

			struct Get
			{
				virtual ShaderPtr operator()(ShaderStageFlags type, const std::filesystem::path& filename, const std::vector<std::string>& defines) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;

			struct Release
			{
				virtual void operator()(ShaderPtr shader) = 0;
			};
			FLAME_GRAPHICS_API static Release& release;
		};

		struct VertexAttributeInfo
		{
			int location = -1;
			int offset = -1;
			Format format = Format_Undefined;
		};

		struct VertexBufferInfo
		{
			std::vector<VertexAttributeInfo> attributes;
			VertexInputRate rate = VertexInputRateVertex;
			uint stride = 0;
		};

		FLAME_GRAPHICS_API UdtInfo* get_vertex_input_ui(const std::filesystem::path& filename, const std::vector<std::string>& defines);

		/*
			if (Enable)
			{
				finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
				finalColor.a   = (srcAlphaBlendFactor * newColor.a  ) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a  );
			}
			else
				finalColor = newColor;

			finalColor = finalColor & colorWriteMask;
		*/
		struct BlendOption
		{
			bool enable = false;
			BlendFactor src_color = BlendFactorZero;
			BlendFactor dst_color = BlendFactorZero;
			BlendFactor src_alpha = BlendFactorZero;
			BlendFactor dst_alpha = BlendFactorZero;
		};

		struct PipelineInfo
		{
			PipelineLayoutPtr layout = nullptr;
			std::vector<ShaderPtr> shaders;
			RenderpassPtr renderpass = nullptr;
			uint subpass_index = 0;
			std::vector<VertexBufferInfo> vertex_buffers;
			PrimitiveTopology primitive_topology = PrimitiveTopologyTriangleList;
			uint patch_control_points = 0;
			bool rasterizer_discard = false;
			PolygonMode polygon_mode = PolygonModeFill;
			CullMode cull_mode = CullModeBack;
			SampleCount sample_count = SampleCount_1;
			bool alpha_to_coverage = false;
			bool depth_test = true;
			bool depth_write = true;
			CompareOp compare_op = CompareOpLess;
			std::vector<BlendOption> blend_options;
			std::vector<DynamicState> dynamic_states;
		};

		struct GraphicsPipeline : PipelineInfo
		{
			std::filesystem::path filename;
			std::vector<std::string> defines;
			bool dynamic_renderpass = true;
			uint ref = 0;

			std::vector<std::pair<uint, void*>> dependencies;

			virtual ~GraphicsPipeline() {}

			virtual void recreate() = 0;

			inline ShaderPtr vert() const
			{
				for (auto s : shaders)
				{
					if (((Shader*)s)->type == ShaderStageVert)
						return s;
				}
				return nullptr;
			}

			inline ShaderPtr frag() const
			{
				for (auto s : shaders)
				{
					if (((Shader*)s)->type == ShaderStageFrag)
						return s;
				}
				return nullptr;
			}

			// vertex input udt info
			inline UdtInfo* vi_ui() const
			{
				auto s = vert();
				if (s)
					return ((Shader*)s)->in_ui;
				return nullptr;
			}

			struct Create
			{
				virtual GraphicsPipelinePtr operator()(const PipelineInfo& info) = 0;
				virtual GraphicsPipelinePtr operator()(const std::string& content, const std::vector<std::string>& defines) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;

			struct Get
			{
				virtual GraphicsPipelinePtr operator()(const std::filesystem::path& filename, const std::vector<std::string>& defines) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;

			struct Release
			{
				virtual void operator()(GraphicsPipelinePtr pipeline) = 0;
			};
			FLAME_GRAPHICS_API static Release& release;
		};

		struct ComputePipeline : PipelineInfo
		{
			std::filesystem::path filename;
			std::vector<std::string> defines;
			uint ref = 0;

			std::vector<std::pair<uint, void*>> dependencies;

			virtual ~ComputePipeline() {}

			struct Create
			{
				virtual ComputePipelinePtr operator()(const PipelineInfo& info) = 0;
				virtual ComputePipelinePtr operator()(const std::string& content, const std::vector<std::string>& defines, const std::string& key = "") = 0;
			};
			FLAME_GRAPHICS_API static Create& create;

			struct Get
			{
				virtual ComputePipelinePtr operator()(const std::filesystem::path& filename, const std::vector<std::string>& defines) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;

			struct Release
			{
				virtual void operator()(ComputePipelinePtr pipeline) = 0;
			};
			FLAME_GRAPHICS_API static Release& release;
		};
	}
}
