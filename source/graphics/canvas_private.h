#include <flame/graphics/canvas.h>

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct BufferPrivate;
		struct ImagePrivate;
		struct ImageviewPrivate;
		struct ImageAtlasPrivate;
		struct FontAtlasPrivate;
		struct FramebufferPrivate;
		struct DescriptorsetPrivate;
		struct CommandbufferPrivate;

		struct CanvasResourcePrivate : CanvasResource
		{
			ImageviewPrivate* _view;
			ImageAtlasPrivate* _atlas;
			Vec2f _white_uv;

			Imageview* get_view() const override { return _view; }
			ImageAtlas* get_atlas() const override { return _atlas; }
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

			DevicePrivate* _d;

			Vec4f _clear_color;

			std::unique_ptr<ImagePrivate> _img_white;
			std::vector<std::unique_ptr<CanvasResourcePrivate>> _resources;
			std::unique_ptr<BufferPrivate> _buf_vtx;
			std::unique_ptr<BufferPrivate> _buf_idx;
			std::vector<std::unique_ptr<FramebufferPrivate>> _fbs;
			std::unique_ptr<DescriptorsetPrivate> _ds;

			Vertex* _vtx_end;
			uint* _idx_end;

			Vec2u _target_size;
			Vec4f _curr_scissor;

			std::vector<Cmd> _cmds;
			uint* _p_vtx_cnt;
			uint* _p_idx_cnt;

			CanvasPrivate(DevicePrivate* d);

			void _set_target(std::span<ImageviewPrivate*> views);

			uint _set_resource(int slot, ImageviewPrivate* v, SamplerPrivate* sp, ImageAtlasPrivate* atlas = nullptr);
			void _add_atlas(ImageAtlasPrivate* a);
			void _add_font(FontAtlasPrivate* f);

			void _add_draw_cmd(int id = -1);
			void _add_vtx(const Vec2f& pos, const Vec2f& uv, const Vec4c& col);
			void _add_idx(uint idx);

			void _stroke(std::span<const Vec2f> points, const Vec4c& col, float thickness);
			void _fill(std::span<const Vec2f> points, const Vec4c& col);
			void _add_text(FontAtlasPrivate* f, std::wstring_view text, uint font_size, const Vec2f& pos, const Vec4c& col);
			void _add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col);

			void _set_scissor(const Vec4f& _scissor);

			void _prepare();
			void _record(CommandbufferPrivate* cb, uint image_index);

			void release() override { delete this; }

			Vec4f get_clear_color() const override { return _clear_color; }
			void set_clear_color(const Vec4c& color) override { _clear_color = Vec4f(color) / 255.f; }

			void set_target(uint views_count, Imageview* const* views) override { _set_target({ (ImageviewPrivate**)views, views_count }); }

			CanvasResource* get_resource(uint slot) override { return _resources[slot].get(); }
			uint set_resource(int slot, Imageview* v, Sampler* sp, ImageAtlas* atlas) override { return _set_resource(slot, (ImageviewPrivate*)v, (SamplerPrivate*)sp, (ImageAtlasPrivate*)atlas); }
			void add_atlas(ImageAtlas* a) override { _add_atlas((ImageAtlasPrivate*)a); }
			void add_font(FontAtlas* f) override { _add_font((FontAtlasPrivate*)f); }

			void stroke(uint points_count, const Vec2f* points, const Vec4c& col, float thickness) override { _stroke( { points, points_count}, col, thickness); }
			void fill(uint points_count, const Vec2f* points, const Vec4c& col) override { _fill({ points, points_count }, col); }
			void add_text(FontAtlas* f, const wchar_t* text, int text_len, uint font_size, const Vec2f& pos, const Vec4c& col) override { _add_text((FontAtlasPrivate*)f, { text, text_len == - 1 ? std::char_traits<wchar_t>::length(text) : text_len }, font_size, pos, col); }
			void add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col) override { _add_image(pos, size, id, uv0, uv1, tint_col); }

			Vec4f get_scissor() const override { return _curr_scissor; }
			void set_scissor(const Vec4f& scissor) override { _set_scissor(scissor); }

			void prepare() override { _prepare(); }
			void record(Commandbuffer* cb, uint image_index) override { _record((CommandbufferPrivate*)cb, image_index); }
		};
	}
}
