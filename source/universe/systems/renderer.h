#pragma once

#include "../system.h"

namespace flame
{
	struct sRendererParms
	{
	};

	struct sRenderer : System
	{
		inline static auto type_name = "flame::sRenderer";
		inline static auto type_hash = ch(type_name);

		sRenderer() :
			System(type_name, type_hash)
		{
		}

		virtual void set_shade_wireframe(bool v) = 0;
		virtual void set_always_update(bool a) = 0;

		/* element res type:
		*	"ImageView"
		*	"ImageAtlas"
		*	"FontAtlas"
		*/

		virtual void* get_element_res(uint idx, char* type) const = 0;
		virtual int set_element_res(int idx, const char* type, void* v) = 0;
		virtual int find_element_res(void* v) const = 0;

		virtual void fill_rect(uint layer, cElementPtr element, const vec2& pos, const vec2& size, const cvec4& color) = 0;
		virtual void stroke(uint layer, cElementPtr element, uint pt_cnt, const vec2* pts, float thickness, const cvec4& color) = 0;
		virtual void stroke_rect(uint layer, cElementPtr element, const vec2& pos, const vec2& size, float thickness, const cvec4& color) = 0;
		virtual void draw_text(uint layer, cElementPtr element, const vec2& pos, uint font_size, uint font_id,
			const wchar_t* text_beg, const wchar_t* text_end, const cvec4& color) = 0;

		virtual int set_texture_res(int idx, graphics::ImageView* tex, graphics::Sampler* sp) = 0;
		virtual int find_texture_res(graphics::ImageView* tex) const = 0;

		virtual int set_material_res(int idx, graphics::Material* mat) = 0;
		virtual int find_material_res(graphics::Material* mat) const = 0;

		virtual int set_mesh_res(int idx, graphics::Mesh* mesh) = 0;
		virtual int find_mesh_res(graphics::Mesh* mesh) const = 0;

		virtual cCameraPtr get_camera() const = 0;
		virtual void set_camera(cCameraPtr camera) = 0;

		virtual void get_sky(graphics::ImageView** out_box, graphics::ImageView** out_irr, 
			graphics::ImageView** out_rad, graphics::ImageView** out_lut, void** out_id) = 0;
		virtual void set_sky(graphics::ImageView* box, graphics::ImageView* irr,
			graphics::ImageView* rad, graphics::ImageView* lut, void* id) = 0;

		virtual void add_light(cNodePtr node, LightType type, const vec3& color, bool cast_shadow) = 0;
		virtual void draw_mesh(cNodePtr node, uint mesh_id, bool cast_shadow) = 0;
		virtual void draw_terrain(cNodePtr node, const uvec2& blocks, uint tess_levels, uint height_map_id, uint normal_map_id, uint material_id) = 0;

		virtual bool is_dirty() const = 0;
		virtual void mark_dirty() = 0;

		virtual void set_targets(uint tar_cnt, graphics::ImageView* const* ivs) = 0;
		virtual void record(uint tar_idx, graphics::CommandBuffer* cb) = 0;

		FLAME_UNIVERSE_EXPORTS static sRenderer* create(void* parms = nullptr);
	};
}
