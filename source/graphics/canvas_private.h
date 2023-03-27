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
				uint idx_cnt;
				DescriptorSetPtr ds;
			};

			GraphicsPipelinePtr pl = nullptr;
			RenderpassPtr rp = nullptr;
			RenderpassPtr rp_load = nullptr;
			VertexBuffer buf_vtx;
			IndexBuffer<> buf_idx;
			FontAtlasPtr main_font_atlas = nullptr;
			ImagePtr main_img = nullptr;
			std::unique_ptr<DescriptorSetT> main_ds;

			std::vector<DrawCmd> draw_cmds;
			std::vector<vec2> path;

			CanvasPrivate(WindowPtr window, bool use_window_targets = true, std::span<ImageViewPtr> targets = {});
			~CanvasPrivate();
			void set_targets(std::span<ImageViewPtr> targets);
			void reset();
			DrawCmd& get_cmd(DescriptorSetPtr ds);

			void path_rect(const vec2& a, const vec2& b);
			void stroke_path(DrawCmd& cmd, float thickness, const cvec4& col, bool closed);
			void fill_path(DrawCmd& cmd, const cvec4& col);
			void stroke(float thickness, const cvec4& col, bool closed);
			void fill(const cvec4& col);

			void add_rect(const vec2& a, const vec2& b, float thickness, const cvec4& col) override;
			void add_rect_filled(const vec2& a, const vec2& b, const cvec4& col) override;
			void add_text(FontAtlasPtr font_atlas, uint font_size, const vec2& pos, std::wstring_view str, const cvec4& col) override;
			void add_image(ImageViewPtr view, const vec2& a, const vec2& b, const vec4& uvs) override;
		};
	}
}
