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
		struct Model;
		struct Mesh;
		struct Sampler;
		struct CommandBuffer;

		enum ResourceType
		{
			ResourceImage,
			ResourceImageAtlas,
			ResourceFontAtlas,
			ResourceTexture,
			ResourceMaterial,
			ResourceModel
		};

		struct ArmatureDeformer
		{
			virtual void release() = 0;

			virtual void set_pose(uint id, const Mat4f& pose) = 0;
			virtual void update(CommandBuffer* cb) = 0;

			FLAME_GRAPHICS_EXPORTS static ArmatureDeformer* create(Device* d, Mesh* mesh);
		};

		struct Canvas
		{
			virtual void release() = 0;

			virtual Device* get_device() const = 0;

			virtual Vec4c get_clear_color() const = 0;
			virtual void set_clear_color(const Vec4c& color) = 0;

			virtual ImageView* get_target(uint idx) const = 0;
			virtual void set_target(uint views_count, ImageView* const* views) = 0;

			virtual void* get_resource(ResourceType type, uint slot, ResourceType* real_type = nullptr) = 0;
			virtual int find_resource(ResourceType type, const char* name) = 0;
			virtual uint set_resource(ResourceType type, int slot /* -1 to find an empty slot */, void* p, const char* name = nullptr) = 0;

			virtual void begin_path() = 0;
			virtual void move_to(const Vec2f& pos) = 0;
			virtual void line_to(const Vec2f& pos) = 0;
			virtual void close_path() = 0;

			virtual void stroke(const Vec4c& col, float thickness, bool aa = false) = 0;
			virtual void fill(const Vec4c& col, bool aa = false) = 0;
			// null text_end means render until '\0'
			virtual void draw_text(uint res_id, const wchar_t* text_beg, const wchar_t* text_end, uint font_size, const Vec4c& col, const Vec2f& pos, const Mat2f& axes = Mat2f(1.f)) = 0;
			virtual void draw_image(uint res_id, uint tile_id, const Vec2f& LT, const Vec2f& RT, const Vec2f& RB, const Vec2f& LB, const Vec2f& uv0 = Vec2f(0.f), const Vec2f& uv1 = Vec2f(1.f), const Vec4c& tint_col = Vec4c(255)) = 0;

			virtual void set_camera(float fovy, float aspect, float zNear, float zFar, const Mat3f& axes, const Vec3f& coord) = 0;

			virtual void draw_mesh(uint mod_id, uint mesh_idx, const Mat4f& transform, const Mat4f& normal_matrix, bool cast_shadow, ArmatureDeformer* deformer = nullptr) = 0;
			virtual void draw_terrain(uint height_tex_id, uint color_tex_id, const Vec2u& size, const Vec3f& extent, const Vec3f& coord, float tess_levels = 32.f) = 0;
			virtual void add_light(LightType type, const Mat4f& transform, const Vec3f& color, bool cast_shadow = false) = 0;

			virtual Vec4f get_scissor() const = 0;
			virtual void set_scissor(const Vec4f& scissor) = 0;

			virtual void add_blur(const Vec4f& range, uint radius) = 0;
			virtual void add_bloom() = 0;

			virtual void prepare() = 0;
			virtual void record(CommandBuffer* cb, uint image_index) = 0;

			FLAME_GRAPHICS_EXPORTS static Canvas* create(Device* d, bool hdr = false, bool msaa_3d = false);
		};
	}
}
