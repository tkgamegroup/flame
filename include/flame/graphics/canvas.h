#pragma once

#include <flame/foundation/foundation.h>
#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct ImageView;
		struct ImageAtlas;
		struct FontAtlas;
		struct Material;
		struct Mesh;
		struct Model;
		struct Sampler;
		struct CommandBuffer;

		enum ShadingType
		{
			ShadingSolid,
			ShadingWireframe
		};

		struct Point3
		{
			vec3 position;
			cvec4 color;
		};

		struct Line3
		{
			Point3 a;
			Point3 b;
		};

		struct RenderPreferences
		{
			FLAME_GRAPHICS_EXPORTS static RenderPreferences* create(Device* device, bool hdr = true, bool msaa_3d = true);
		};

		struct ArmatureDeformer
		{
			virtual void release() = 0;

			virtual void set_pose(uint id, const mat4& pose) = 0;

			FLAME_GRAPHICS_EXPORTS static ArmatureDeformer* create(RenderPreferences* preferences, Mesh* mesh);
		};

		struct ElementResource
		{
			ImageView* iv;
			ImageAtlas* ia;
			FontAtlas* fa;
		};

		struct Canvas
		{
			virtual void release() = 0;

			virtual RenderPreferences* get_preferences() const = 0;

			virtual void set_shading(ShadingType type) = 0;
			virtual void set_shadow(float distance, uint csm_levels, float csm_factor) = 0;

			virtual cvec4 get_clear_color() const = 0;
			virtual void set_clear_color(const cvec4& color) = 0;

			virtual ImageView* get_output(uint idx) const = 0;
			virtual void set_output(uint views_count, ImageView* const* views) = 0;

			virtual ElementResource get_element_resource(uint slot) = 0;
			virtual int find_element_resource(const char* name) = 0;
			virtual uint set_element_resource(int slot /* -1 to find an empty slot */, ElementResource r, const char* name = nullptr) = 0;

			virtual ImageView* get_texture_resource(uint slot) = 0;
			virtual int find_texture_resource(const char* name) = 0;
			virtual uint set_texture_resource(int slot /* -1 to find an empty slot */, ImageView* iv, Sampler* sp, const char* name = nullptr) = 0;

			virtual Material* get_material_resource(uint slot) = 0;
			virtual int find_material_resource(const char* name) = 0;
			virtual uint set_material_resource(int slot /* -1 to find an empty slot */, Material* mat, const char* name = nullptr) = 0;

			virtual Model* get_model_resource(uint slot) = 0;
			virtual int find_model_resource(const char* name) = 0;
			virtual uint set_model_resource(int slot /* -1 to find an empty slot */, Model* mod, const char* name = nullptr) = 0;

			virtual void begin_path() = 0;
			virtual void move_to(const vec2& pos) = 0;
			virtual void line_to(const vec2& pos) = 0;
			virtual void close_path() = 0;

			virtual void stroke(const cvec4& col, float thickness, bool aa = false) = 0;
			virtual void fill(const cvec4& col, bool aa = false) = 0;

			virtual void draw_image(uint res_id, uint tile_id, const vec2& pos, const vec2& size, const mat2& axes = mat2(1.f), const vec2& uv0 = vec2(0.f), const vec2& uv1 = vec2(1.f), const cvec4& tint_col = cvec4(255)) = 0;
			// null text_end means render until '\0'
			virtual void draw_text(uint res_id, const wchar_t* text_beg, const wchar_t* text_end, uint font_size, const cvec4& col, const vec2& pos, const mat2& axes = mat2(1.f)) = 0;

			virtual void set_camera(float fovy, float aspect, float zNear, float zFar, const mat3& dirs, const vec3& coord) = 0;
			virtual void set_sky(ImageView* box, ImageView* irr, ImageView* rad) = 0;

			virtual void draw_mesh(uint mod_id, uint mesh_idx, const mat4& transform, bool cast_shadow = true, ArmatureDeformer* deformer = nullptr) = 0;
			virtual void draw_terrain(const uvec2& blocks, const vec3& scale, const vec3& coord, float tess_levels, uint height_tex_id, uint normal_tex_id, uint mat_id) = 0;
			virtual void add_light(LightType type, const mat3& dirs, const vec3& color, bool cast_shadow = false) = 0;

			virtual void draw_lines(uint lines_count, const Line3* lines) = 0;

			virtual Rect get_scissor() const = 0;
			virtual void set_scissor(const Rect& scissor) = 0;

			virtual void add_blur(const Rect& range, uint radius) = 0;
			virtual void add_bloom() = 0;

			virtual void prepare() = 0;
			virtual void record(CommandBuffer* cb, uint image_index) = 0;

			FLAME_GRAPHICS_EXPORTS static Canvas* create(RenderPreferences* preferences);
		};
	}
}
