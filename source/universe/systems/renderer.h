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

		virtual void set_always_update(bool a) = 0;
		virtual void set_render_type(RenderType type) = 0;
		virtual void get_shadow_props(uint* dir_levels, float* dir_dist, float* pt_dist) = 0;
		virtual void set_shadow_props(uint dir_levels, float dir_dist, float pt_dist) = 0;

		virtual graphics::ImageView* get_element_res(uint idx) const = 0;
		virtual int set_element_res(int idx, graphics::ImageView* iv) = 0;
		virtual int find_element_res(graphics::ImageView* iv) const = 0;

		virtual void fill(uint layer, uint pt_cnt, const vec2* pts, const cvec4& color) = 0;
		virtual void stroke(uint layer, uint pt_cnt, const vec2* pts, float thickness, const cvec4& color, bool closed = false) = 0;
		virtual void draw_glyphs(uint layer, uint cnt, const graphics::GlyphDraw* glyphs, uint res_id, const cvec4& color) = 0;
		virtual void draw_image(uint layer, const vec2* pts, uint res_id, const vec4& uvs, const cvec4& tint_color) = 0;

		virtual int set_texture_res(int idx, graphics::ImageView* tex, graphics::Sampler* sp) = 0;
		virtual int find_texture_res(graphics::ImageView* tex) const = 0;

		virtual int set_material_res(int idx, graphics::Material* mat) = 0;
		virtual int find_material_res(graphics::Material* mat) const = 0;

		virtual int set_mesh_res(int idx, graphics::Mesh* mesh) = 0;
		virtual int find_mesh_res(graphics::Mesh* mesh) const = 0;

		virtual cCameraPtr get_camera() const = 0;
		virtual void set_camera(cCameraPtr camera) = 0;

		virtual void* get_sky_id() = 0;
		virtual void set_sky(graphics::ImageView* box, graphics::ImageView* irr,
			graphics::ImageView* rad, graphics::ImageView* lut, const vec3& fog_color, float intensity, void* id) = 0;

		virtual void add_light(const mat4& mat, LightType type, const vec3& color, bool cast_shadow) = 0;
		virtual uint add_mesh_transform(const mat4& mat, const mat3& nor) = 0;
		virtual uint add_mesh_armature(uint bones_count, const mat4* bones) = 0;
		virtual void draw_mesh(uint idx, uint mesh_id, ShadingFlags flags = ShadingMaterial) = 0;
		virtual void add_mesh_occluder(uint idx, uint mesh_id) = 0;
		virtual void draw_terrain(const vec3& coord, const vec3& extent, const uvec2& blocks, uint tess_levels, uint height_map_id, uint normal_map_id, 
			uint material_id, ShadingFlags flags = ShadingMaterial) = 0;
		virtual void draw_particles(uint count, graphics::Particle* partcles, uint res_id) = 0;

		virtual void draw_lines(uint count, graphics::Line* lines) = 0;

		virtual bool is_dirty() const = 0;
		virtual void mark_dirty() = 0;

		virtual void set_targets(uint tar_cnt, graphics::ImageView* const* ivs) = 0;
		virtual void record(uint tar_idx, graphics::CommandBuffer* cb) = 0;

		/* for debug use */
		virtual uint get_shadow_count(LightType t) = 0;
		virtual void get_shadow_matrices(LightType t, uint idx, mat4* dst) = 0;

		FLAME_UNIVERSE_EXPORTS static sRenderer* create(void* parms = nullptr);
	};
}
