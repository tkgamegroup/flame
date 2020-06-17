#include <flame/graphics/canvas.h>

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct BufferPrivate;
		struct ImagePrivate;
		struct FramebufferPrivate;
		struct DescriptorsetPrivate;
		struct CommandbufferPrivate;

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

			DevicePrivate* d;

			Vec4f clear_color;

			std::unique_ptr<ImagePrivate> img_white;
			std::vector<Resource> resources;
			std::unique_ptr<BufferPrivate> buf_vtx;
			std::unique_ptr<BufferPrivate> buf_idx;
			std::vector<std::unique_ptr<FramebufferPrivate>> fbs;
			std::unique_ptr<DescriptorsetPrivate> ds;

			Vertex* vtx_end;
			uint* idx_end;

			Vec2u target_size;
			Vec4f curr_scissor;

			std::vector<Cmd> cmds;

			CanvasPrivate(DevicePrivate* d);

			void release() override;

			Vec4f get_clear_color() const override;
			void set_clear_color(const Vec4f& color) override;

			void set_target(uint view_count, Imageview* const* views) override;
			void _set_target(const std::vector<Imageview*>& views);

			Resource get_resource(uint slot) override;
			uint set_resource(int slot, Imageview* v, Sampler* sp, ImageAtlas* atlas) override;
			void add_atlas(ImageAtlas* a) override;
			void add_font(FontAtlas* f) override;

			Vec4f get_scissor() const override;
			void set_scissor(const Vec4f& _scissor) override;

			void stroke(uint point_count, const Vec2f* points, const Vec4c& col, float thickness) override;
			void fill(uint point_count, const Vec2f* points, const Vec4c& col) override;
			void add_text(FontAtlas* f, const wchar_t* text_begin, const wchar_t* text_end, uint font_size, const Vec2f& _pos, const Vec4c& col) override;
			void add_image(const Vec2f& _pos, const Vec2f& size, uint id, const Vec2f& _uv0, const Vec2f& _uv1, const Vec4c& tint_col) override;

			void prepare() override;
			void record(Commandbuffer* cb, uint image_index) override;
		};
	}
}
