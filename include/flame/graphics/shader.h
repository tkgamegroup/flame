#pragma once

#include <flame/graphics/graphics.h>

#include <vector>

namespace flame
{
	struct UdtInfo;

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
			UdtInfo* buffer_udt;

			DescriptorBinding() :
				binding(-1),
				type(DescriptorUniformBuffer),
				count(1),
				buffer_udt(nullptr)
			{
			}

			DescriptorBinding(uint binding, DescriptorType$ type, uint count = 1, const std::string& name = "", UdtInfo* buffer_udt = nullptr) :
				binding(binding),
				type(type),
				count(count),
				name(name),
				buffer_udt(buffer_udt)
			{
			}
		};

		struct Descriptorlayout
		{
			const DescriptorBinding& get_binding(uint binding);

			FLAME_GRAPHICS_EXPORTS static Descriptorlayout* create(Device* d, const std::vector<void*>& bindings);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorlayout* l);
		};

		struct Descriptorset
		{
			FLAME_GRAPHICS_EXPORTS Descriptorlayout* layout();

			FLAME_GRAPHICS_EXPORTS void set_buffer(uint binding, uint index, Buffer* b, uint offset = 0, uint range = 0);
			FLAME_GRAPHICS_EXPORTS void set_image(uint binding, uint index, Imageview* v, Sampler* sampler);

			FLAME_GRAPHICS_EXPORTS static Descriptorset* create(Descriptorpool* p, Descriptorlayout* l);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorset* s);
		};

		struct Pipelinelayout
		{
			FLAME_GRAPHICS_EXPORTS static Pipelinelayout* create(Device* d, const std::vector<void*>& descriptorsetlayouts, uint push_constant_size, UdtInfo* push_constant_udt = nullptr);
			FLAME_GRAPHICS_EXPORTS static void destroy(Pipelinelayout* p);
		};

		struct VertexInputAttribute
		{
			uint location;
			uint buffer_id;
			uint offset;
			Format$ format;
			std::string name;

			VertexInputAttribute() :
				location(0),
				buffer_id(0),
				offset(0),
				format(Format_Undefined)
			{
			}

			VertexInputAttribute(uint location, uint buffer_id, uint offset, Format$ format, const std::string& name = "") :
				location(location),
				buffer_id(buffer_id),
				offset(offset),
				format(format),
				name(name)
			{
			}
		};

		struct VertexInputBuffer
		{
			uint id;
			uint stride;
			VertexInputRate$ rate;

			VertexInputBuffer() :
				id(0),
				stride(0),
				rate(VertexInputRateVertex)
			{
			}

			VertexInputBuffer(uint id, uint stride, VertexInputRate$ rate = VertexInputRateVertex) :
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
			PrimitiveTopology$ primitive_topology;
			uint patch_control_points;

			VertexInputInfo() :
				primitive_topology(PrimitiveTopologyTriangleList),
				patch_control_points(0)
			{
			}
		};

		struct StageInOut
		{
			uint location;
			Format$ format;
			std::string name;

			StageInOut() :
				location(0),
				format(Format_Undefined)
			{
			}

			StageInOut(uint location, Format$ format, const std::string& name = "") :
				location(location),
				format(format),
				name(name)
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
				cull_mode(CullModeNone)
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
			BlendFactor$ blend_src_color;
			BlendFactor$ blend_dst_color;
			BlendFactor$ blend_src_alpha;
			BlendFactor$ blend_dst_alpha;
			bool dual_src;

			OutputAttachmentInfo() :
				location(0),
				format(Format_R8G8B8A8_UNORM),
				blend_enable(false),
				blend_src_color(BlendFactorOne),
				blend_dst_color(BlendFactorZero),
				blend_src_alpha(BlendFactorOne),
				blend_dst_alpha(BlendFactorZero),
				dual_src(false)
			{
			}

			OutputAttachmentInfo(uint location, Format$ format, const std::string& name) :
				location(location),
				format(format),
				name(name),
				blend_enable(false),
				blend_src_color(BlendFactorOne),
				blend_dst_color(BlendFactorZero),
				blend_src_alpha(BlendFactorOne),
				blend_dst_alpha(BlendFactorZero),
				dual_src(false)
			{
			}

			OutputAttachmentInfo(uint location, Format$ format, const std::string& name, BlendFactor$ sc, BlendFactor$ dc, BlendFactor$ sa, BlendFactor$ da) :
				location(location),
				format(format),
				name(name),
				blend_enable(true),
				blend_src_color(sc),
				blend_dst_color(dc),
				blend_src_alpha(sa),
				blend_dst_alpha(da)
			{
				judge_dual();
			}

			void judge_dual()
			{
				dual_src = is_blend_factor_dual(blend_src_color) || 
					is_blend_factor_dual(blend_dst_color) ||
					is_blend_factor_dual(blend_src_alpha) ||
					is_blend_factor_dual(blend_dst_alpha);
			}
		};

		inline ShaderStage$ shader_stage_from_filename(const std::wstring& filename)
		{
			auto ext = std::filesystem::path(filename).extension();
			if (ext == L".vert")
				return ShaderStageVert;
			else if (ext == L".tesc")
				return ShaderStageTesc;
			else if (ext == L".tese")
				return ShaderStageTese;
			else if (ext == L".geom")
				return ShaderStageGeom;
			else if (ext == L".frag")
				return ShaderStageFrag;
			else if (ext == L".comp")
				return ShaderStageComp;
			return ShaderStageNone;
		}

		FLAME_GRAPHICS_EXPORTS Mail<std::string> get_shader_autogen_code(ShaderStage$ stage, const std::vector<void*>* inputs = nullptr, const std::vector<void*>* outputs = nullptr, Pipelinelayout* pll = nullptr);

		struct Shader
		{
			FLAME_GRAPHICS_EXPORTS static Shader* create(Device* d, const std::wstring& filename, const std::string& prefix, const std::vector<void*>* inputs = nullptr, const std::vector<void*>* outputs = nullptr, Pipelinelayout* pll = nullptr, bool autogen_code = false);
			// for vertex shader, inputs are the VertexInputAttributeInfos, for fragment shader, outputs are the OutputAttachmentInfos, otherwise, inputs and outputs are StageInOutInfos
			// if autogen_code, inputs, outputs and pll are used to generate the code, otherwise, just the validation
			FLAME_GRAPHICS_EXPORTS static void destroy(Shader* s);
		};

		struct Pipeline
		{
			PipelineType type;

			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* d, const std::vector<void*>& shaders, Pipelinelayout* pll, Renderpass* rp, uint subpass_idx, 
				VertexInputInfo* vi = nullptr, const Vec2u& vp = Vec2u(0), RasterInfo* raster = nullptr, SampleCount$ sc = SampleCount_1, DepthInfo* depth = nullptr,
				const std::vector<void*>& outputs = {}, const std::vector<uint>& dynamic_states = {});
			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* d, Shader* compute_shader, Pipelinelayout* pll);
			FLAME_GRAPHICS_EXPORTS static  void destroy(Pipeline* p);
		};
	}
}
