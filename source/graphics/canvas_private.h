#include <flame/graphics/canvas.h>

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct BufferPrivate;
		struct ImagePrivate;
		struct ImageViewPrivate;
		struct ImageAtlasPrivate;
		struct FontAtlasPrivate;
		struct FramebufferPrivate;
		struct DescriptorSetPrivate;
		struct CommandBufferPrivate;

		struct CanvasResourcePrivate : CanvasResource
		{
			ImageViewPrivate* view;
			ImageAtlasPrivate* image_atlas = nullptr;
			FontAtlasPrivate* font_atlas = nullptr;
			Vec2f white_uv = Vec2f(0.5f);

			ImageView* get_view() const override { return _view; }
			ImageAtlas* get_image_atlas() const override { return _image_atlas; }
			FontAtlas* get_font_atlas() const override { return _font_atlas; }
			Vec2f get_white_uv() const override { return _white_uv; }
		};

		struct CanvasPrivate : Canvas
		{
			enum CmdType
			{
				CmdDrawElement,
				CmdSetScissor
			};

			struct Cmd
			{
				CmdType type;
				union
				{
					struct
					{
						uint id;
						uint vtx_cnt;
						uint idx_cnt;
					}draw_data;
					Vec4f scissor;
				}v;
			};

			struct Vertex
			{
				Vec2f pos;
				Vec2f uv;
				Vec4c col;
			};

			DevicePrivate* device;

			Vec4f clear_color;

			std::unique_ptr<ImagePrivate> img_white;
			std::vector<std::unique_ptr<CanvasResourcePrivate>> resources;
			std::unique_ptr<BufferPrivate> buf_vtx;
			std::unique_ptr<BufferPrivate> buf_idx;
			std::vector<std::unique_ptr<FramebufferPrivate>> fbs;
			std::unique_ptr<DescriptorSetPrivate> ds;

			Vertex* vtx_end;
			uint* idx_end;

			Vec2u target_size;
			Vec4f curr_scissor;

			std::vector<Cmd> cmds;
			uint* p_vtx_cnt;
			uint* p_idx_cnt;

			CanvasPrivate(DevicePrivate* d);

			void _set_target(std::span<ImageViewPrivate*> views);

			uint _set_resource(int slot, ImageViewPrivate* v, SamplerPrivate* sp, ImageAtlasPrivate* image_atlas = nullptr, FontAtlasPrivate* font_atlas = nullptr);
			void _add_atlas(ImageAtlasPrivate* a);
			void _add_font(FontAtlasPrivate* f);

			void _add_draw_cmd(int id = -1);
			void _add_vtx(const Vec2f& pos, const Vec2f& uv, const Vec4c& col);
			void _add_idx(uint idx);

			void _stroke(std::span<const Vec2f> points, const Vec4c& col, float thickness, bool aa = false);
			void _fill(std::span<const Vec2f> points, const Vec4c& col, bool aa = false);
			void _add_text(uint res_id, const wchar_t* text, uint font_size, const Vec2f& pos, const Vec4c& col);
			void _add_image(uint res_id, uint tile_id, const Vec2f& pos, const Vec2f& size, Vec2f uv0, Vec2f uv1, const Vec4c& tint_col);

			void _set_scissor(const Vec4f& _scissor);

			void _prepare();
			void _record(CommandBufferPrivate* cb, uint image_index);

			void release() override { delete this; }

			Vec4f get_clear_color() const override { return _clear_color; }
			void set_clear_color(const Vec4c& color) override { _clear_color = Vec4f(color) / 255.f; }

			void set_target(uint views_count, ImageView* const* views) override { _set_target({ (ImageViewPrivate**)views, views_count }); }

			CanvasResource* get_resource(uint slot) override { return _resources[slot].get(); }
			uint set_resource(int slot, ImageView* v, Sampler* sp, ImageAtlas* image_atlas, FontAtlas* font_atlas) override { return _set_resource(slot, (ImageViewPrivate*)v, (SamplerPrivate*)sp, (ImageAtlasPrivate*)image_atlas, (FontAtlasPrivate*)font_atlas); }
			void add_atlas(ImageAtlas* a) override { _add_atlas((ImageAtlasPrivate*)a); }
			void add_font(FontAtlas* f) override { _add_font((FontAtlasPrivate*)f); }

			void stroke(uint points_count, const Vec2f* points, const Vec4c& col, float thickness, bool aa) override { _stroke( { points, points_count}, col, thickness, aa); }
			void fill(uint points_count, const Vec2f* points, const Vec4c& col, bool aa) override { _fill({ points, points_count }, col, aa); }
			void add_text(uint res_id, const wchar_t* text, uint size, const Vec2f& pos, const Vec4c& col) override { _add_text(res_id, text, size, pos, col); }
			void add_image(uint res_id, uint tile_id, const Vec2f& pos, const Vec2f& size, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col) override { _add_image(res_id, tile_id, pos, size, uv0, uv1, tint_col); }

			Vec4f get_scissor() const override { return _curr_scissor; }
			void set_scissor(const Vec4f& scissor) override { _set_scissor(scissor); }

			void prepare() override { _prepare(); }
			void record(CommandBuffer* cb, uint image_index) override { _record((CommandBufferPrivate*)cb, image_index); }
		};
	}
}
