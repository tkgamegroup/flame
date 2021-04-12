#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct DescriptorPool
		{
			virtual void release() = 0;

			FLAME_GRAPHICS_EXPORTS static DescriptorPool* get_default(Device* device);
			FLAME_GRAPHICS_EXPORTS static DescriptorPool* create(Device* device);
		};

		struct DescriptorBindingInfo
		{
			DescriptorType type;
			uint count = 1;
			const char* name = "";
		};

		struct DescriptorSetLayout
		{
			virtual void release() = 0;

			virtual uint get_bindings_count() const = 0;
			virtual void get_binding(uint binding, DescriptorBindingInfo* ret) const = 0;
			virtual int find_binding(const char* name) const = 0;

			FLAME_GRAPHICS_EXPORTS static DescriptorSetLayout* create(Device* device, uint bindings_count, const DescriptorBindingInfo* bindings);
			FLAME_GRAPHICS_EXPORTS static DescriptorSetLayout* get(Device* device, const wchar_t* filename);
		};

		struct DescriptorSet
		{
			virtual void release() = 0;

			virtual DescriptorSetLayoutPtr get_layout() const = 0;

			virtual void set_buffer(uint binding, uint index, BufferPtr b, uint offset = 0, uint range = 0) = 0;
			virtual void set_image(uint binding, uint index, ImageViewPtr v, SamplerPtr sp) = 0;

			FLAME_GRAPHICS_EXPORTS static DescriptorSet* create(DescriptorPool* pool, DescriptorSetLayout* layout);
		};

		struct PipelineLayout
		{
			virtual void release() = 0;

			FLAME_GRAPHICS_EXPORTS static PipelineLayout* create(Device* device, uint descriptorlayouts_count, DescriptorSetLayout* const* descriptor_layouts, uint push_constant_size);
			FLAME_GRAPHICS_EXPORTS static PipelineLayout* get(Device* device, const wchar_t* filename);
		};

		struct Shader
		{
			virtual void release() = 0;

			virtual const wchar_t* get_filename() const = 0;

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

			FLAME_GRAPHICS_EXPORTS static Shader* get(Device* device, const wchar_t* filename, const char* defines, const char* substitutes);
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
			RenderpassPtr renderpass;
			uint subpass_index = 0;
			uint vertex_buffers_count = 0;
			const VertexBufferInfo* vertex_buffers = nullptr;
			PrimitiveTopology primitive_topology = PrimitiveTopologyTriangleList;
			uint patch_control_points = 0;
			bool depth_clamp = false;
			PolygonMode polygon_mode = PolygonModeFill;
			CullMode cull_mode = CullModeBack;
			bool depth_test = true;
			bool depth_write = true;
			CompareOp compare_op = CompareOpLess;
			uint blend_options_count = 0;
			const BlendOption* blend_options = nullptr;
			uint dynamic_states_count = 0;
			const uint* dynamic_states = nullptr;
		};

		struct Pipeline
		{
			virtual void release() = 0;

			virtual PipelineType get_type() const = 0;

			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* device, uint shaders_count, Shader* const* shaders, PipelineLayout* pll, const GraphicsPipelineInfo& info);
			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* device, Shader* compute_shader, PipelineLayout* pll);
			FLAME_GRAPHICS_EXPORTS static Pipeline* get(Device* device, const wchar_t* filename);
		};
	}
}
