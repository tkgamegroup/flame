#pragma once

#include <flame/graphics/graphics.h>

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct Buffer;
		struct Sampler;

		struct Descriptorpool
		{
			FLAME_GRAPHICS_EXPORTS static Descriptorpool* create(Device* d);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorpool* p);
		};

		struct DescriptorBinding
		{
			uint binding;
			DescriptorType$ type;
			uint count;
			std::string name;

			DescriptorBinding() :
				binding(-1),
				type(DescriptorUniformBuffer),
				count(1)
			{
			}

			DescriptorBinding(uint binding, DescriptorType$ type, uint count = 1, const std::string& name = "") :
				binding(binding),
				type(type),
				count(count),
				name(name)
			{
			}
		};

		struct Descriptorlayout
		{
			FLAME_GRAPHICS_EXPORTS static Descriptorlayout* create(Device* d, const std::vector<void*>& bindings);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorlayout* l);
		};

		struct Descriptorset
		{
			FLAME_GRAPHICS_EXPORTS void set_buffer(uint binding, uint index, Buffer* b, uint offset = 0, uint range = 0);
			FLAME_GRAPHICS_EXPORTS void set_image(uint binding, uint index, Imageview* v, Sampler* sampler);

			FLAME_GRAPHICS_EXPORTS static Descriptorset* create(Descriptorpool* p, Descriptorlayout* l);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorset* s);
		};

		struct Pipelinelayout
		{
			FLAME_GRAPHICS_EXPORTS static Pipelinelayout* create(Device* d, const std::vector<void*>& descriptorsetlayouts, uint push_constant_size, uint push_constant_udt_name_hash = 0);
			FLAME_GRAPHICS_EXPORTS static void destroy(Pipelinelayout* p);
		};

		struct Shader
		{
			FLAME_GRAPHICS_EXPORTS static Shader* create(Device* d, const std::wstring& filename, const std::string& prefix, const std::vector<void*>& inputs = {}, const std::vector<void*>& outputs = {}, Pipelinelayout* pll = nullptr, bool autogen_code = false);
			// for vertex shader, inputs are the VertexInputAttributeInfos, for fragment shader, outputs are the OutputAttachmentInfos, otherwise, inputs and outputs are StageInOutInfos
			// if autogen_code, inputs, outputs and pll are used to generate the code, otherwise, just the validation
			FLAME_GRAPHICS_EXPORTS static void destroy(Shader* s);
		};

		struct StageInOutInfo
		{
			uint location;
			Format$ format;
			std::string name;

			StageInOutInfo(uint location, Format$ format, const std::string& name = "") :
				location(location),
				format(format),
				name(name)
			{
			}
		};

		struct VertexInputAttributeInfo
		{
			uint location;
			uint buffer_id;
			uint offset;
			Format$ format;
			std::string name;

			VertexInputAttributeInfo(uint location, uint buffer_id, uint offset, Format$ format, const std::string& name = "") :
				location(location),
				buffer_id(buffer_id),
				offset(offset),
				format(format),
				name(name)
			{
			}
		};

		struct VertexInputBufferInfo
		{
			uint id;
			uint stride;
			VertexInputRate rate;

			VertexInputBufferInfo(uint id, uint stride, VertexInputRate rate = VertexInputRateVertex) :
				id(id),
				stride(stride),
				rate(rate)
			{
			}
		};

		struct VertexInputInfo
		{
			std::vector<void*> attribs;
			std::vector<void*> buffers;
			PrimitiveTopology primitive_topology;
			uint patch_control_points;

			VertexInputInfo() :
				primitive_topology(PrimitiveTopologyTriangleList),
				patch_control_points(0)
			{
			}
		};

		struct RasterInfo
		{
			bool depth_clamp;
			PolygonMode polygon_mode;
			CullMode cull_mode;

			RasterInfo() :
				depth_clamp(false),
				polygon_mode(PolygonModeFill),
				cull_mode(CullModeBack)
			{
			}
		};

		struct DepthInfo
		{
			bool test;
			bool write;
			CompareOp compare_op;

			DepthInfo() :
				test(false),
				write(false),
				compare_op(CompareOpLess)
			{
			}
		};

		struct OutputAttachmentInfo
		{
			uint location;
			Format$ format;
			std::string name;
			bool blend_enable;
			BlendFactor blend_src_color;
			BlendFactor blend_dst_color;
			BlendFactor blend_src_alpha;
			BlendFactor blend_dst_alpha;

			OutputAttachmentInfo() :
				location(0),
				format(Format_R8G8B8A8_UNORM)
			{
				blend_enable = false;
				blend_src_color = BlendFactorOne;
				blend_dst_color = BlendFactorZero;
				blend_src_alpha = BlendFactorOne;
				blend_dst_alpha = BlendFactorZero;
			}

			OutputAttachmentInfo(uint location, Format$ format, const std::string& name) :
				location(location),
				format(format),
				name(name)
			{
				blend_enable = false;
				blend_src_color = BlendFactorOne;
				blend_dst_color = BlendFactorZero;
				blend_src_alpha = BlendFactorOne;
				blend_dst_alpha = BlendFactorZero;
			}

			OutputAttachmentInfo(uint location, Format$ format, const std::string& name, BlendFactor sc, BlendFactor dc, BlendFactor sa, BlendFactor da) :
				location(location),
				format(format),
				name(name)
			{
				blend_enable = true;
				blend_src_color = sc;
				blend_dst_color = dc;
				blend_src_alpha = sa;
				blend_dst_alpha = da;
			}
		};

		struct Pipeline
		{
			PipelineType type;

			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* d, const std::vector<void*>& shaders, Pipelinelayout* pll, Renderpass* rp, uint subpass_idx, 
				VertexInputInfo* vi = nullptr, const Vec2u& vp = Vec2u(0), RasterInfo* raster = nullptr, SampleCount$ sc = SampleCount_1, DepthInfo* depth = nullptr,
				const std::vector<void*>& output_states = {}, const std::vector<uint>& dynamic_states = {});
			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* d, Shader* compute_shader, Pipelinelayout* pll);
			FLAME_GRAPHICS_EXPORTS static  void destroy(Pipeline* p);
		};
	}
}
