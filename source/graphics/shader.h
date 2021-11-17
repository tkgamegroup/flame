#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct DescriptorPool
		{
			virtual ~DescriptorPool() {}

			FLAME_GRAPHICS_EXPORTS static DescriptorPoolPtr get_default(DevicePtr device = nullptr);
			FLAME_GRAPHICS_EXPORTS static DescriptorPoolPtr create(DevicePtr device);
		};

		struct DescriptorBinding
		{
			DescriptorType type = Descriptor_Max;
			uint count = 1;
			std::string name;

			UdtInfo* ti = nullptr;
		};

		struct DescriptorSetLayout
		{
			std::vector<DescriptorBinding> bindings;

			std::filesystem::path filename;

			virtual ~DescriptorSetLayout() {}

			inline int find_binding(std::string_view name) const
			{
				for (auto i = 0; i < bindings.size(); i++)
				{
					if (bindings[i].name == name)
						return i;
				}
				return -1;
			}

			FLAME_GRAPHICS_EXPORTS static DescriptorSetLayoutPtr create(DevicePtr device, std::span<DescriptorBinding> bindings);
			FLAME_GRAPHICS_EXPORTS static DescriptorSetLayoutPtr get(DevicePtr device, const std::filesystem::path& filename);
		};

		struct DescriptorSet
		{
			DescriptorSetLayoutPtr layout;

			virtual ~DescriptorSet() {}

			virtual void set_buffer(uint binding, uint index, BufferPtr buf, uint offset = 0, uint range = 0) = 0;
			virtual void set_image(uint binding, uint index, ImageViewPtr iv, SamplerPtr sp) = 0;
			virtual void update() = 0;

			FLAME_GRAPHICS_EXPORTS static DescriptorSetPtr create(DescriptorPoolPtr pool, DescriptorSetLayoutPtr layout);
		};

		struct PipelineLayout
		{
			std::vector<DescriptorSetLayoutPtr> descriptor_set_layouts;

			TypeInfoDataBase db;
			UdtInfo* pc_ti = nullptr;
			uint pc_sz = 0;

			std::filesystem::path filename;

			virtual ~PipelineLayout() {}

			FLAME_GRAPHICS_EXPORTS static PipelineLayoutPtr create(DevicePtr device, std::span<DescriptorSetLayoutPtr> descriptor_layouts, uint push_constant_size);
			FLAME_GRAPHICS_EXPORTS static PipelineLayoutPtr get(DevicePtr device, const std::filesystem::path& filename);
		};

		struct Shader
		{
			ShaderStageFlags type = ShaderStageNone;
			std::filesystem::path filename;
			std::vector<std::string> defines;
			std::vector<std::pair<std::string, std::string>> substitutes;

			virtual ~Shader() {}

			inline static std::vector<std::string> format_defines(const std::string& defines)
			{
				std::vector<std::string> ret;
				auto sp = SUS::split(defines);
				for (auto& s : sp)
				{
					SUS::trim(s);
					if (!s.empty())
						ret.push_back(s);
				}
				return ret;
			}

			inline static std::vector<std::pair<std::string, std::string>> format_substitutes(const std::string& substitutes)
			{
				std::vector<std::pair<std::string, std::string>> ret;
				auto sp = SUS::split(substitutes);
				for (auto i = 0; i < (int)sp.size() - 1; i += 2)
				{
					SUS::trim(sp[i]);
					SUS::trim(sp[i + 1]);
					if (!sp[i].empty() && !sp[i + 1].empty())
						ret.emplace_back(sp[i], sp[i + 1]);
				}
				return ret;
			}

			FLAME_GRAPHICS_EXPORTS static ShaderPtr get(DevicePtr device, const std::filesystem::path& filename, const std::vector<std::string>& defines, const std::vector<std::pair<std::string, std::string>>& substitutes);
		};

		struct VertexAttributeInfo
		{
			uint location;
			int offset = -1;
			Format format;
		};

		struct VertexBufferInfo
		{
			uint attributes_count = 0;
			const VertexAttributeInfo* attributes = nullptr;
			VertexInputRate rate = VertexInputRateVertex;
			uint stride = 0;
		};
		
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

		struct GraphicsPipelineInfo
		{
			uint shaders_count;
			Shader** shaders;
			PipelineLayout* layout;
			RenderpassPtr renderpass;
			uint subpass_index = 0;
			uint vertex_buffers_count = 0;
			const VertexBufferInfo* vertex_buffers = nullptr;
			PrimitiveTopology primitive_topology = PrimitiveTopologyTriangleList;
			uint patch_control_points = 0;
			PolygonMode polygon_mode = PolygonModeFill;
			CullMode cull_mode = CullModeBack;
			SampleCount sample_count = SampleCount_1;
			bool alpha_to_coverage = false;
			bool depth_test = true;
			bool depth_write = true;
			CompareOp compare_op = CompareOpLess;
			uint blend_options_count = 0;
			const BlendOption* blend_options = nullptr;
			uint dynamic_states_count = 0;
			const uint* dynamic_states = nullptr;
		};

		struct ComputePipelineInfo
		{
			Shader* shader;
			PipelineLayout* layout;
		};

		struct GraphicsPipeline
		{
			PipelineType type;
			PipelineLayoutPtr layout;
			std::vector<ShaderPtr> shaders;

			std::filesystem::path filename;

			virtual ~GraphicsPipeline() {}

			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* device, const GraphicsPipelineInfo& info);
			FLAME_GRAPHICS_EXPORTS static Pipeline* get(Device* device, const wchar_t* filename);
		};

		struct ComputePipeline
		{
			PipelineType type;
			PipelineLayoutPtr layout;
			ShaderPtr shader;

			std::filesystem::path filename;

			virtual ~ComputePipeline() {}

			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* device, const ComputePipelineInfo& info);
			FLAME_GRAPHICS_EXPORTS static Pipeline* get(Device* device, const wchar_t* filename);
		};
	}
}
