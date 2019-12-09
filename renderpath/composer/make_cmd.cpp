#include <flame/foundation/foundation.h>
#include <flame/foundation/bitmap.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/all.h>

using namespace flame;
using namespace graphics;

namespace flame
{
	struct MakeCmd$;

	struct MakeCmd$
	{
		AttributeP<std::vector<void*>> cbs$i;
		AttributeP<void> rnf$i;
		AttributeD<uint> image_idx$i;
		AttributeP<void> pll$i;
		AttributeP<void> pl$i;
		AttributeP<void> ds$i;

		Vec4c clear_color;

		std::vector<Imageview*> imgs;

		uint set_image(int index, Imageview* v, Filter filter, Atlas* atlas)
		{
			if (index == -1)
			{
				assert(v);
				for (auto i = 1; i < imgs.size(); i++)
				{
					if (imgs[i].first == (Imageview*)white_iv$i.v)
					{
						index = i;
						break;
					}
				}
				assert(index != -1);
			}
			if (!v)
			{
				v = (Imageview*)white_iv$i.v;
				atlas = nullptr;
			}
			((Descriptorset*)ds$i.v)->set_image(0, index, v, filter);
			imgs[index] = std::make_pair(v, atlas);
			return index;
		}

		__declspec(dllexport) void update$()
		{
			auto rnf = (RenderpassAndFramebuffer*)rnf$i.v;
			auto img_idx = image_idx$i.v;
			auto cb = (Commandbuffer*)(*cbs$i.v)[img_idx];
			auto vtx_buf = (Buffer*)vtx_buf$i.v;
			auto idx_buf = (Buffer*)idx_buf$i.v;
			auto pll = (Pipelinelayout*)pll$i.v;
			auto pl_element = (Pipeline*)pl_element$i.v;
			auto pl_text_lcd = (Pipeline*)pl_text_lcd$i.v;
			auto pl_text_sdf = (Pipeline*)pl_text_sdf$i.v;
			auto ds = (Descriptorset*)ds$i.v;

			if (rnf)
			{
				auto fb = (Framebuffer*)rnf->framebuffers()[img_idx];

				cb->begin();
				auto cv = rnf->clearvalues();
				cv->set(0, clear_color);
				cb->begin_renderpass(fb, cv);
				if (idx_end != idx_buf->mapped)
				{
					cb->set_viewport(curr_scissor);
					cb->set_scissor(curr_scissor);
					cb->bind_vertexbuffer(vtx_buf, 0);
					cb->bind_indexbuffer(idx_buf, IndiceTypeUint);

					cb->bind_descriptorset(ds, 0, pll);

					auto vtx_off = 0;
					auto idx_off = 0;
					for (auto& cmd : cmds)
					{
						switch (cmd.type)
						{
						case CmdDrawElement:
							if (cmd.v.draw_data.idx_cnt > 0)
							{
								cb->bind_pipeline(pl_element);
								cb->draw_indexed(cmd.v.draw_data.idx_cnt, idx_off, vtx_off, 1, cmd.v.draw_data.id);
								vtx_off += cmd.v.draw_data.vtx_cnt;
								idx_off += cmd.v.draw_data.idx_cnt;
							}
							break;
						case CmdDrawTextLcd:
							if (cmd.v.draw_data.idx_cnt > 0)
							{
								cb->bind_pipeline(pl_text_lcd);
								cb->draw_indexed(cmd.v.draw_data.idx_cnt, idx_off, vtx_off, 1, cmd.v.draw_data.id);
								vtx_off += cmd.v.draw_data.vtx_cnt;
								idx_off += cmd.v.draw_data.idx_cnt;
							}
							break;
						case CmdDrawTextSdf:
							if (cmd.v.draw_data.idx_cnt > 0)
							{
								cb->bind_pipeline(pl_text_sdf);
								cb->draw_indexed(cmd.v.draw_data.idx_cnt, idx_off, vtx_off, 1, cmd.v.draw_data.id);
								vtx_off += cmd.v.draw_data.vtx_cnt;
								idx_off += cmd.v.draw_data.idx_cnt;
							}
							break;
						case CmdSetScissor:
							cb->set_scissor(cmd.v.scissor);
							break;
						}
					}
				}
				cb->end_renderpass();
				cb->end();

				vtx_end = (Vertex*)vtx_buf->mapped;
				idx_end = (uint*)idx_buf->mapped;
				cmds.clear();
			}
			else
			{
				cb->begin();
				cb->end();
			}
		}
	};

	void CanvasPrivate::set_clear_color(const Vec4c& col)
	{
		thiz->clear_color = col;
	}

	Imageview* CanvasPrivate::get_image(uint index)
	{
		return thiz->imgs[index].first;
	}

	uint CanvasPrivate::set_image(int index, Imageview* v, Filter filter, Atlas* atlas)
	{
		return thiz->set_image(index, v, filter, atlas);
	}

	void CanvasPrivate::add_image(const Vec2f& pos, const Vec2f& size, uint id, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col, bool repeat)
	{
		thiz->add_image(pos, size, id, uv0, uv1, tint_col, repeat);
	}
}

