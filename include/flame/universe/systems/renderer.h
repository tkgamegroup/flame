#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct cElement;
	struct cNode;
	struct cCamera;

	struct sRendererParms
	{
	};

	struct sRenderer : System
	{
		inline static auto type_name = "flame::sRenderer";
		inline static auto type_hash = ch(type_name);

		enum ElementResType
		{
			ElementResImage,
			ElementResAtlas,
			ElementResFont
		};

		sRenderer() :
			System(type_name, type_hash)
		{
		}

		virtual void set_shade_wireframe(bool v) = 0;
		virtual void set_always_update(bool a) = 0;

		virtual void* get_element_res(uint idx, ElementResType* type) const = 0;
		virtual int set_element_res(int idx, ElementResType type, void* v) = 0;
		virtual int find_element_res(void* v) const = 0;

		virtual void fill_rect(uint layer, cElement* element, const vec2& pos, const vec2& size, const cvec4& color) = 0;
		virtual void stroke_rect(uint layer, cElement* element, const vec2& pos, const vec2& size, float thickness, const cvec4& color) = 0;
		virtual void draw_text(uint layer, cElement* element, const vec2& pos, uint font_size, uint font_id,
			const wchar_t* text_beg, const wchar_t* text_end, const cvec4& color) = 0;

		virtual int set_texture_res(int idx, graphics::ImageView* view) = 0;
		virtual int find_texture_res(graphics::ImageView* view) const = 0;

		virtual int set_material_res(int idx, graphics::Material* mat) = 0;
		virtual int find_material_res(graphics::Material* mat) const = 0;

		virtual int set_mesh_res(int idx, graphics::model::Mesh* mesh) = 0;
		virtual int find_mesh_res(graphics::model::Mesh* mesh) const = 0;

		virtual cCamera* get_camera() const = 0;
		virtual void set_camera(cCamera* camera) = 0;

		virtual void draw_mesh(cNode* node, uint mesh_id) = 0;

		virtual bool is_dirty() const = 0;
		virtual void mark_dirty() = 0;

		virtual void record_drawing_commands(uint tar_idx, graphics::CommandBuffer* cb) = 0;

		FLAME_UNIVERSE_EXPORTS static sRenderer* create(void* parms = nullptr);
	};
}
