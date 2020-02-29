#pragma once

#include <flame/graphics/graphics.h>

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct Buffer;
		struct Renderpass;
		struct Sampler;
		struct Descriptorset;

		struct Descriptorpool
		{
			FLAME_GRAPHICS_EXPORTS static Descriptorpool* create(Device* d);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorpool* p);
		};

		struct DescriptorBinding
		{
			DescriptorType type;
			uint count;
			const char* name;
			Buffer* buffer;
			Imageview* view;
			Sampler* sampler;
		};

		struct Descriptorlayout
		{
			FLAME_GRAPHICS_EXPORTS uint binding_count() const;
			FLAME_GRAPHICS_EXPORTS const DescriptorBinding& get_binding(uint binding) const;
			FLAME_GRAPHICS_EXPORTS Descriptorset* default_set() const;

			FLAME_GRAPHICS_EXPORTS static Descriptorlayout* create(Device* d, uint binding_count, DescriptorBinding* const* bindings, Descriptorpool* default_set_pool = nullptr);
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
			FLAME_GRAPHICS_EXPORTS static Pipelinelayout* create(Device* d, uint descriptorlayout_count, Descriptorlayout* const* descriptorlayouts, uint push_constant_size);
			FLAME_GRAPHICS_EXPORTS static void destroy(Pipelinelayout* p);
		};

		struct VertexInputAttribute
		{
			const char* name;
			Format format;
		};

		struct VertexInputBuffer
		{
			uint attribute_count;
			VertexInputAttribute* const* attributes;
			VertexInputRate rate;
		};

		struct VertexInputInfo
		{
			uint buffer_count;
			VertexInputBuffer* const* buffers;
			PrimitiveTopology primitive_topology;
			uint patch_control_points;
		};

		struct RasterInfo
		{
			bool depth_clamp;
			PolygonMode polygon_mode;
			CullMode cull_mode;
		};

		struct DepthInfo
		{
			bool test;
			bool write;
			CompareOp compare_op;
		};

		inline std::wstring shader_stage_name(ShaderStage s)
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

		inline ShaderStage shader_stage_from_ext(const std::wstring& extension)
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

			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* d, const wchar_t* shader_dir, uint shader_count, const wchar_t* const* shader_filenames /* filename[:prefix] */, Pipelinelayout* pll, Renderpass* rp, uint subpass_idx,
				VertexInputInfo* vi = nullptr, const Vec2u& vp = Vec2u(0), RasterInfo* raster = nullptr, SampleCount sc = SampleCount_1, DepthInfo* depth = nullptr,
				uint dynamic_state_count = 0, const uint* dynamic_states = nullptr);
			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* d, const wchar_t* shader_dir, const wchar_t* compute_shader_filename /* filename[:prefix] */, Pipelinelayout* pll);
			FLAME_GRAPHICS_EXPORTS static  void destroy(Pipeline* p);
		};
	}
}
