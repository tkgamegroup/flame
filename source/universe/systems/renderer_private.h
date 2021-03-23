#pragma once

#include <flame/graphics/graphics.h>
#include <flame/universe/systems/renderer.h>

namespace flame
{
	struct Window;

	namespace graphics
	{
		struct Device;
		struct Buffer;
		struct Image;
		struct Renderpass;
		struct Framebuffer;
		struct Pipeline;
		struct DescriptorSet;
		struct CommandBuffer;
		struct Swapchain;
	}

	struct EntityPrivate;
	struct cElementPrivate;
	struct cNodePrivate;
	struct cCameraPrivate;

	template <class T>
	struct GeometryBuffer
	{
		uint capacity;
		graphics::AccessFlags access;
		T* pstag = nullptr;
		uint stag_num = 0;

		FlmPtr<graphics::Buffer> buf;
		FlmPtr<graphics::Buffer> stagbuf;

		void rebuild();
		void create(graphics::Device* d, graphics::BufferUsageFlags usage, uint capacity);
		void push(uint cnt, const T* p);
		T* stag(uint cnt);
		void upload(graphics::CommandBuffer* cb);
	};

	template <class T>
	struct PileBuffer
	{
		uint capacity;
		graphics::AccessFlags access;
		uint n0 = 0;
		uint n1 = 0;

		FlmPtr<graphics::Buffer> buf;
		FlmPtr<graphics::Buffer> stagbuf;

		void create(graphics::Device* d, graphics::BufferUsageFlags usage, uint capacity);
		T* alloc(uint n);
		void free(T* p);
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
		FlmPtr<graphics::Pipeline> pipeline;
	};

	struct sRendererBridge : sRenderer
	{
		void draw_text(uint layer, cElement* element, const vec2& pos, uint font_size, uint font_id,
			const wchar_t* text_beg, const wchar_t* text_end, const cvec4& color) override;
		void fill_rect(uint layer, cElement* element, const vec2& pos, const vec2& size, const cvec4& color) override;
		void stroke_rect(uint layer, cElement* element, const vec2& pos, const vec2& size, float thickness, const cvec4& color) override;
	};

	struct sRendererPrivate : sRendererBridge
	{
		struct ElementRes
		{
			ElementResType type;
			void* v;
		};

		struct MaterialRes
		{
			graphics::Material* mat;
			uint texs[4];
		};

		struct MeshRes
		{
			graphics::Mesh* mesh = nullptr;
			bool arm;
			uint vtx_off;
			uint vtx_cnt;
			uint idx_off;
			uint idx_cnt;
		};

		bool wireframe = false;
		bool always_update = false;

		Window* window;

		graphics::Canvas* canvas = nullptr;
		cCameraPrivate* camera = nullptr;

		graphics::Device* device;
		graphics::Swapchain* swapchain;

		std::vector<FlmPtr<graphics::Framebuffer>> fb_targets;

		// ==== element drawing ====

		std::vector<ElementRes> element_reses;

		GeometryBuffer<ElementVertex>	buf_element_vtx;
		GeometryBuffer<uint>			buf_element_idx;
		FlmPtr<graphics::Image>			img_wht;
		FlmPtr<graphics::DescriptorSet>	ds_element;

		graphics::Pipeline*				pl_element;
		// =========================

		// ==== node drawing ====

		std::vector<MaterialRes> mat_reses;
		std::vector<MeshRes> mesh_reses;
		
		FlmPtr<graphics::Framebuffer> fb_def;

		PileBuffer<MeshVertex>		buf_mesh_vtx;
		PileBuffer<uint>			buf_mesh_idx;
		PileBuffer<ArmMeshVertex>	buf_arm_mesh_vtx;
		PileBuffer<uint>			buf_arm_mesh_idx;

		FlmPtr<graphics::Image> img_dep;
		FlmPtr<graphics::Image> img_def_geo0; // albedo, metallic
		FlmPtr<graphics::Image> img_def_geo1; // normal, roughness

		std::vector<MaterialPipeline>	pl_mats[MaterialUsageCount];
		// ======================

		// ==== post ====

		FlmPtr<graphics::Pipeline>		pl_gamma;

		// ==============

		bool dirty = true;

		vec2 tar_size;

		cElementPrivate* last_element;
		bool last_element_changed;

		Rect element_drawing_scissor;
		std::vector<ElementDrawCmd> element_drawing_layers[128];

		std::vector<uint> node_drawing_meshes;

		sRendererPrivate(sRendererParms* parms);

		void set_targets();

		void set_shade_wireframe(bool v) override { wireframe = v; }
		void set_always_update(bool a) override { always_update = a; }

		void* get_element_res(uint idx, ElementResType* type) const override;
		int set_element_res(int idx, ElementResType type, void* v) override;
		int find_element_res(void* v) const override;

		int set_material_res(int idx, graphics::Material* mesh) override;
		int find_material_res(graphics::Material* mesh) const override;

		int set_mesh_res(int idx, graphics::Mesh* mesh) override;
		int find_mesh_res(graphics::Mesh* mesh) const override;

		void fill_rect(uint layer, cElementPrivate* element, const vec2& pos, const vec2& size, const cvec4& color);
		void stroke_rect(uint layer, cElementPrivate* element, const vec2& pos, const vec2& size, float thickness, const cvec4& color);
		void draw_text(uint layer, cElementPrivate* element, const vec2& pos, uint font_size, uint font_id,
			const wchar_t* text_beg, const wchar_t* text_end, const cvec4& color);

		graphics::Pipeline* get_material_pipeline(MaterialUsage usage, const std::filesystem::path& mat, const std::string& defines);
		void release_material_pipeline(MaterialUsage usage, graphics::Pipeline* pl);

		cCamera* get_camera() const override { return (cCamera*)camera; }
		void set_camera(cCamera* c) override { camera = (cCameraPrivate*)c; }

		bool is_dirty() const override { return always_update || dirty; }
		void mark_dirty() override { dirty = true; }

		uint element_render(uint layer, cElementPrivate* element);
		void node_render(cNodePrivate* node);

		void record_element_drawing_commands(uint tar_idx, graphics::CommandBuffer* cb);
		void record_node_drawing_commands(uint tar_idx, graphics::CommandBuffer* cb);
		void record_drawing_commands(uint tar_idx, graphics::CommandBuffer* cb) override;

		void on_added() override;

		void update() override;
	};

	inline void sRendererBridge::fill_rect(uint layer, cElement* element, const vec2& pos, const vec2& size, const cvec4& color)
	{
		((sRendererPrivate*)this)->fill_rect(layer, (cElementPrivate*)element, pos, size, color);
	}

	inline void sRendererBridge::stroke_rect(uint layer, cElement* element, const vec2& pos, const vec2& size, float thickness, const cvec4& color)
	{
		((sRendererPrivate*)this)->stroke_rect(layer, (cElementPrivate*)element, pos, size, thickness, color);
	}

	inline void sRendererBridge::draw_text(uint layer, cElement* element, const vec2& pos, uint font_size, uint font_id,
		const wchar_t* text_beg, const wchar_t* text_end, const cvec4& color)
	{
		((sRendererPrivate*)this)->draw_text(layer, (cElementPrivate*)element, pos, font_size, font_id, text_beg, text_end, color);
	}
}
