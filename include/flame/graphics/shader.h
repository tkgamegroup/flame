#pragma once

#include <flame/graphics/graphics.h>

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct Buffer;
		struct Sampler;
		struct Descriptorset;

		struct Descriptorpool
		{
			FLAME_GRAPHICS_EXPORTS static Descriptorpool* create(Device* d);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorpool* p);
		};

		struct DescriptorBindingBase
		{
			DescriptorType$ type;
			uint count;
			std::string name;
		};

		struct DescriptorBufferBinding : DescriptorBindingBase
		{
			Buffer* buffer;

			DescriptorBufferBinding() :
				buffer(nullptr)
			{
			}
		};

		struct DescriptorImageBinding : DescriptorBindingBase
		{
			Imageview* view;
			Sampler* sampler;

			DescriptorImageBinding() :
				view(nullptr)
			{
			}
		};

		struct Descriptorlayout
		{
			FLAME_GRAPHICS_EXPORTS uint binding_count() const;
			FLAME_GRAPHICS_EXPORTS const DescriptorBindingBase* get_binding(uint binding) const;
			FLAME_GRAPHICS_EXPORTS Descriptorset* default_set() const;

			FLAME_GRAPHICS_EXPORTS static Descriptorlayout* create(Device* d, const std::vector<void*>& bindings, Descriptorpool* default_set_pool = nullptr);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorlayout* l);
		};

		struct Descriptorset
		{
			FLAME_GRAPHICS_EXPORTS Descriptorlayout* layout();

			FLAME_GRAPHICS_EXPORTS void set_buffer(uint binding, uint index, Buffer* b, uint offset = 0, uint range = 0);
			FLAME_GRAPHICS_EXPORTS void set_image(uint binding, uint index, Imageview* v, Sampler* sampler);
			FLAME_GRAPHICS_EXPORTS void set_image(uint binding, uint index, Imageview* v, Filter filter);

			FLAME_GRAPHICS_EXPORTS static Descriptorset* create(Descriptorpool* p, Descriptorlayout* l);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorset* s);
		};

		struct Pipelinelayout
		{
			FLAME_GRAPHICS_EXPORTS static Pipelinelayout* create(Device* d, const std::vector<void*>& descriptorsetlayouts, uint push_constant_size);
			FLAME_GRAPHICS_EXPORTS static void destroy(Pipelinelayout* p);
		};

		struct VertexInputAttribute
		{
			std::string name;
			Format$ format;
		};

		struct VertexInputBuffer
		{
			std::vector<void*> attributes;
			VertexInputRate$ rate;

			VertexInputBuffer() :
				rate(VertexInputRateVertex)
			{
			}

			VertexInputBuffer(uint id, uint stride, VertexInputRate$ rate = VertexInputRateVertex) :
				rate(rate)
			{
			}
		};

		struct VertexInputInfo
		{
			std::vector<void*> buffers;
			PrimitiveTopology$ primitive_topology;
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

		inline std::wstring shader_stage_name(ShaderStage$ s)
		{
			switch (s)
			{
			case ShaderStageVert:
				return L"vert";
			case ShaderStageTesc:
				return L"tesc";
			case ShaderStageTese:
				return L"tese";
			case ShaderStageGeom:
				return L"geom";
			case ShaderStageFrag:
				return L"frag";
			case ShaderStageComp:
				return L"comp";
			}
		}

		inline ShaderStage$ shader_stage_from_ext(const std::filesystem::path& extension)
		{
			if (extension == L".vert")
				return ShaderStageVert;
			else if (extension == L".tesc")
				return ShaderStageTesc;
			else if (extension == L".tese")
				return ShaderStageTese;
			else if (extension == L".geom")
				return ShaderStageGeom;
			else if (extension == L".frag")
				return ShaderStageFrag;
			else if (extension == L".comp")
				return ShaderStageComp;
			return ShaderStageNone;
		}

		// 'i_' or 'o_' will be eliminated to verify between stages
		// for blend factor:
		// 0 - zero
		// 1 - one
		// sc - src color
		// dc - dst color
		// sa - src alpha
		// da - dst alpha
		// 1msa - one minus src alpha
		// s1c - src1 color
		// 1ms1c - one minus src1 color

		struct Pipeline
		{
			PipelineType type;

			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* d, const std::vector<std::wstring>& shader_filenames /* filename[:prefix] */, Pipelinelayout* pll, Renderpass* rp, uint subpass_idx,
				VertexInputInfo* vi = nullptr, const Vec2u& vp = Vec2u(0), RasterInfo* raster = nullptr, SampleCount$ sc = SampleCount_1, DepthInfo* depth = nullptr,
				const std::vector<uint>& dynamic_states = {});
			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* d, const std::wstring& compute_shader_filename /* filename[:prefix] */, Pipelinelayout* pll);
			FLAME_GRAPHICS_EXPORTS static  void destroy(Pipeline* p);
		};
	}
}
