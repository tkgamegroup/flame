#pragma once

#include "../../graphics/command.h"
#include "renderer.h"

namespace DSL_render_data
{
	struct RenderData;
}

namespace DSL_transform
{
	struct Transforms;
}

namespace DSL_material
{
	struct MaterialInfos;
}

namespace DSL_light
{
	struct LightData;
	struct GridLights;
}

namespace flame
{
	template <class T>
	struct SequentialBuffer
	{
		uint capacity;
		graphics::AccessFlags access;
		T* pstag = nullptr;
		uint stag_num = 0;

		UniPtr<graphics::Buffer> buf;
		UniPtr<graphics::Buffer> stagbuf;

		void rebuild();
		void create(graphics::Device* device, graphics::BufferUsageFlags usage, uint capacity);
		void push(uint cnt, const T* p);
		T* stag(uint cnt);
		void upload(graphics::CommandBuffer* cb);
	};

	template <class T>
	struct SparseBuffer
	{
		uint capacity;
		graphics::AccessFlags access;
		uint n0 = 0;
		uint n1 = 0;

		UniPtr<graphics::Buffer> buf;
		UniPtr<graphics::Buffer> stagbuf;
		uint stag_capacity;
		T* pstag = nullptr;

		void create(graphics::Device* device, graphics::BufferUsageFlags usage, uint capacity);
		T* alloc(uint n);
		void free(T* p);
		void upload(graphics::CommandBuffer* cb);
	};

	template <class T>
	struct StorageBuffer
	{
		T* pstag = nullptr;

		UniPtr<graphics::Buffer> buf;
		UniPtr<graphics::Buffer> stagbuf;

		std::vector<graphics::BufferCopy> cpies;

		void create(graphics::Device* device, graphics::BufferUsageFlags usage);
		void upload(graphics::CommandBuffer* cb);
	};

	struct ElementVertex
	{
		vec2 pos;
		vec2 uv;
		cvec4 col;

		void set(const vec2& _pos, const vec2& _uv, const cvec4& _col)
		{
			pos = _pos;
			uv = _uv;
			col = _col;
		}
	};

	struct MeshVertex
	{
		vec3 pos;
		vec2 uv;
		vec3 normal;
	};

	struct ArmMeshVertex : MeshVertex
	{
		ivec4 ids;
		vec4 weights;
	};

	struct ElementDrawCmd
	{
		enum Type
		{
			Fill,
			Stroke,
			Text,
			Scissor
		};

		Type type;
		uint res_id;
		std::vector<vec2> points;
		std::wstring text;
		cvec4 color;
		vec4 misc;
	};

	enum MaterialUsage
	{
		MaterialForMesh,
		MaterialForMeshArmature,
		MaterialForMeshShadow,
		MaterialForMeshShadowArmature,
		MaterialForTerrain,

		MaterialUsageCount
	};

	struct MaterialPipeline
	{
		std::filesystem::path mat;
		std::vector<std::string> defines;
		uint ref_count = 1;
		UniPtr<graphics::Pipeline> pipeline;
	};

	struct sRendererPrivate : sRenderer
	{
		struct ElementRes
		{
			ElementResType type;
			void* v;
		};

		struct MaterialRes
		{
			graphics::Material* mat;
			int texs[4];
			graphics::Pipeline* pls[MaterialUsageCount] = {};

			graphics::Pipeline* get_pl(sRendererPrivate* thiz, MaterialUsage u);

		};

		struct MeshRes
		{
			graphics::model::Mesh* mesh = nullptr;
			bool arm;
			uint vtx_off;
			uint vtx_cnt;
			uint idx_off;
			uint idx_cnt;
			uint mat_id;
		};

		bool wireframe = false;
		bool always_update = false;

		graphics::Device* device;

		graphics::Renderpass* rp_rgba8c;
		graphics::Renderpass* rp_rgba8;
		std::vector<UniPtr<graphics::Framebuffer>> fb_tars;
		vec2 tar_sz;

		// ==== element drawing ====

		std::vector<ElementRes> element_reses;

		Rect						element_drawing_scissor;
		std::vector<ElementDrawCmd> element_drawing_layers[128];

		SequentialBuffer<ElementVertex>	buf_element_vtx;
		SequentialBuffer<uint>			buf_element_idx;
		UniPtr<graphics::Image>			img_wht;
		UniPtr<graphics::DescriptorSet>	ds_element;

		graphics::Pipeline*				pl_element;

		// =========================

		// ==== node drawing ====

		cCameraPrivate* camera = nullptr;

		std::vector<graphics::ImageView*> tex_reses;
		std::vector<MaterialRes> mat_reses;
		std::vector<MeshRes> mesh_reses;

		std::vector<std::vector<std::pair<uint, uint>>> node_drawing_meshes;

		SequentialBuffer<graphics::DrawIndexedIndirectCommand>	buf_indirs;

		SparseBuffer<MeshVertex>	buf_mesh_vtx;
		SparseBuffer<uint>			buf_mesh_idx;
		SparseBuffer<ArmMeshVertex>	buf_arm_mesh_vtx;
		SparseBuffer<uint>			buf_arm_mesh_idx;

		StorageBuffer<DSL_render_data::RenderData>	buf_render_data;
		UniPtr<graphics::DescriptorSet>				ds_render_data;
		StorageBuffer<DSL_transform::Transforms>	buf_transform;
		uint										transform_idx = 0;
		UniPtr<graphics::DescriptorSet>				ds_transform;
		StorageBuffer<DSL_material::MaterialInfos>	buf_material;
		UniPtr<graphics::DescriptorSet>				ds_material;

		UniPtr<graphics::Image> img_back;
		UniPtr<graphics::Image> img_dep;
		UniPtr<graphics::Image> img_alb_met; // albedo, metallic
		UniPtr<graphics::Image> img_nor_rou; // normal, roughness

		StorageBuffer<DSL_light::LightData>			buf_light_data;
		StorageBuffer<DSL_light::GridLights>		buf_grid_light;
		std::vector<UniPtr<graphics::Image>>		img_dir_maps;
		std::vector<UniPtr<graphics::Image>>		img_pt_maps;
		UniPtr<graphics::DescriptorSet>				ds_light;

		UniPtr<graphics::Framebuffer> fb_def;

		std::vector<MaterialPipeline>	pl_mats[MaterialUsageCount];
		graphics::Pipeline*				pl_defe_shad;
		UniPtr<graphics::DescriptorSet>	ds_defe_shad;

		// ======================

		// ==== post ====

		UniPtr<graphics::DescriptorSet>	ds_back;
		graphics::Pipeline*				pl_gamma;

		// ==============

		bool dirty = true;

		sRendererPrivate(sRendererParms* parms);

		void set_shade_wireframe(bool v) override { wireframe = v; }
		void set_always_update(bool a) override { always_update = a; }

		void* get_element_res(uint idx, ElementResType* type) const override;
		int set_element_res(int idx, ElementResType type, void* v) override;
		int find_element_res(void* v) const override;

		void fill_rect(uint layer, cElementPtr element, const vec2& pos, const vec2& size, const cvec4& color) override;
		void stroke_rect(uint layer, cElementPtr element, const vec2& pos, const vec2& size, float thickness, const cvec4& color) override;
		void draw_text(uint layer, cElementPtr element, const vec2& pos, uint font_size, uint font_id, const wchar_t* text_beg, const wchar_t* text_end, const cvec4& color) override;

		int set_texture_res(int idx, graphics::ImageView* tex, graphics::Sampler* sp) override;
		int find_texture_res(graphics::ImageView* tex) const override;

		int set_material_res(int idx, graphics::Material* mat) override;
		int find_material_res(graphics::Material* mat) const override;

		int set_mesh_res(int idx, graphics::model::Mesh* mesh) override;
		int find_mesh_res(graphics::model::Mesh* mesh) const override;

		graphics::Pipeline* get_material_pipeline(MaterialUsage usage, const std::filesystem::path& mat, const std::string& defines);
		void release_material_pipeline(MaterialUsage usage, graphics::Pipeline* pl);

		cCameraPtr get_camera() const override { return camera; }
		void set_camera(cCameraPtr c) override { camera = c; }

		void draw_mesh(cNodePtr node, uint mesh_id) override;

		bool is_dirty() const override { return always_update || dirty; }
		void mark_dirty() override { dirty = true; }

		uint element_render(uint layer, cElementPrivate* element);
		void node_render(cNodePrivate* node);

		void set_targets(uint tar_cnt, graphics::ImageView* const* ivs) override;
		void record(uint tar_idx, graphics::CommandBuffer* cb) override;

		void on_added() override;

		void update() override;
	};
}
