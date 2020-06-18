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
			virtual void release() = 0;

			FLAME_GRAPHICS_EXPORTS static Descriptorpool* create(Device* d);
		};

		struct DescriptorBindingInfo
		{
			DescriptorType type;
			uint count;
			const char* name;
		};

		struct DescriptorBinding
		{
			virtual uint get_index() const = 0;
			virtual DescriptorType get_type() const = 0;
			virtual uint get_count() const = 0;
			virtual const char* get_name() const = 0;
		};

		struct Descriptorlayout
		{
			virtual void release() = 0;

			virtual uint get_bindings_count() const = 0;
			virtual DescriptorBinding* get_binding(uint binding) const = 0;
			virtual Descriptorset* get_default_set() const = 0;

			FLAME_GRAPHICS_EXPORTS static Descriptorlayout* create(Device* d, uint bindings_count, const DescriptorBindingInfo* bindings, bool create_default_set = false);
		};

		struct Descriptorset
		{
			virtual void release() = 0;

			virtual Descriptorlayout* get_layout() const = 0;

			virtual void set_buffer(uint binding, uint index, Buffer* b, uint offset = 0, uint range = 0) = 0;
			virtual void set_image(uint binding, uint index, Imageview* v, Sampler* sampler) = 0;

			FLAME_GRAPHICS_EXPORTS static Descriptorset* create(Descriptorpool* p, Descriptorlayout* l);
		};

		struct Pipelinelayout
		{
			virtual void release() = 0;

			FLAME_GRAPHICS_EXPORTS static Pipelinelayout* create(Device* d, uint descriptorlayouts_count, Descriptorlayout* const* descriptorlayouts, uint push_constant_size);
		};

		struct VertexInputAttribute
		{
			const char* name;
			Format format;
		};

		struct VertexInputBuffer
		{
			uint attributes_count;
			const VertexInputAttribute* attributes;
			VertexInputRate rate;

			VertexInputBuffer() :
				attributes_count(0),
				attributes(nullptr),
				rate(VertexInputRateVertex)
			{
			}
		};

		struct VertexInputInfo
		{
			uint buffer_count;
			const VertexInputBuffer* buffers;
			PrimitiveTopology primitive_topology;
			uint patch_control_points;

			VertexInputInfo() :
				buffer_count(0),
				buffers(nullptr),
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

		// stage variables:
		//  'i_' or 'o_' will be eliminated to verify between stages

		// blend factors:
		//  0 - zero
		//  1 - one
		//  sc - src color
		//  dc - dst color
		//  sa - src alpha
		//  da - dst alpha
		//  1msa - one minus src alpha
		//  s1c - src1 color
		//  1ms1c - one minus src1 color

		struct Shader
		{
			virtual void release() = 0;

			virtual const wchar_t* get_filename() const = 0;
			virtual const char* get_prefix() const = 0;

			FLAME_GRAPHICS_EXPORTS static Shader* create(const wchar_t* filename, const char* prefix);
		};

		struct Pipeline
		{
			virtual void release() = 0;

			virtual PipelineType get_type() const = 0;

			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* d, const wchar_t* shader_dir, uint shaders_count, 
				Shader* const* shaders , Pipelinelayout* pll, Renderpass* rp, uint subpass_idx, 
				VertexInputInfo* vi = nullptr, const Vec2u& vp = Vec2u(0), RasterInfo* raster = nullptr, 
				SampleCount sc = SampleCount_1, DepthInfo* depth = nullptr,
				uint dynamic_states_count = 0, const uint* dynamic_states = nullptr);
			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* d, const wchar_t* shader_dir, const wchar_t* compute_shader_filename /* filename[:prefix] */, Pipelinelayout* pll);
		};
	}
}
