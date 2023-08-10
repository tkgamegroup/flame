#pragma once

#include "canvas.h"
#include "buffer_private.h"
#include "shader_private.h"
#include "command_private.h"
#include "extension.h"

namespace flame
{
	namespace graphics
	{
		struct CanvasPrivate : Canvas
		{
			struct DrawVert
			{
				vec2  pos;
				vec2  uv;
				cvec4 col;
			};

			struct DrawCmd
			{
				enum Type
				{
					SetScissor,
					DrawBmp,
					DrawSdf
				};

				Type type = DrawBmp;
				uint idx_cnt = 0;
				DescriptorSetPtr ds = nullptr;

				union
				{
					struct
					{
						vec4 rect;
					}scissor;
					struct
					{
					}bmp;
					struct
					{
						float scale;
						float thickness;
						float border;
					}sdf;
				}data;
			};

			GraphicsPipelinePtr pl = nullptr;
			GraphicsPipelinePtr pl_sdf = nullptr;
			PipelineResourceManager prm;
			RenderpassPtr rp = nullptr;
			RenderpassPtr rp_load = nullptr;
			VertexBuffer buf_vtx;
			IndexBuffer<> buf_idx;
			FontAtlasPtr default_font_atlas = nullptr;
			ImagePtr main_img = nullptr;
			std::unique_ptr<DescriptorSetT> main_ds;

			std::stack<Rect> scissor_stack;
			std::vector<DrawCmd> draw_cmds;
			std::vector<vec2> path;

			CanvasPrivate(WindowPtr window);
			~CanvasPrivate();

			void create_rp(Format format);
			void set_targets(std::span<ImageViewPtr> targets) override;
			void bind_window_targets() override;
			void reset();
			DrawCmd& get_bmp_cmd(DescriptorSetPtr ds);
			DrawCmd& get_sdf_cmd(DescriptorSetPtr ds, float sdf_scale, float thickness, float border);

			void push_scissor(const Rect& rect) override;
			void pop_scissor() override;

			void path_rect(const vec2& a, const vec2& b);
			DrawVert* stroke_path(DrawCmd& cmd, float thickness, const cvec4& col, bool closed);
			DrawVert* fill_path(DrawCmd& cmd, const cvec4& col);
			void stroke(float thickness, const cvec4& col, bool closed);
			void fill(const cvec4& col);

			void add_rect(const vec2& a, const vec2& b, float thickness, const cvec4& col) override;
			void add_rect_filled(const vec2& a, const vec2& b, const cvec4& col) override;
			void add_text(FontAtlasPtr font_atlas, uint font_size, const vec2& pos, std::wstring_view str, const cvec4& col, float thickness, float border) override;
			void add_image(ImageViewPtr view, const vec2& a, const vec2& b, const vec4& uvs, const cvec4& tint_col) override;
			void add_image_rotated(ImageViewPtr view, const vec2& a, const vec2& b, const vec4& uvs, const cvec4& tint_col, float angle) override;
		};
	}
}
