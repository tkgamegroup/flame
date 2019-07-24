#include <flame/graphics/renderpath.h>
#include "device_private.h"
#include "renderpass_private.h"
#include "commandbuffer_private.h"

#include <memory>

#if defined(FLAME_VULKAN)
#include <spirv_glsl.hpp>
#endif

namespace flame
{
	namespace graphics
	{
		struct RenderpathPrivate : Renderpath
		{
			uint image_count;
			RenderpassPrivate* rp;
			std::vector<std::pair<FramebufferPrivate*, std::vector<Imageview*> /* imageviews that need to be destroyed */>> fbs;
			ClearvaluesPrivate* cv;
			std::vector<CommandbufferPrivate*> cbs;

			RenderpathPrivate(Device* d, const RenderpathInfo& info)
			{
				RenderpassInfo rp_info;
				std::vector<std::tuple<RenderpathPassTargetType, void*, std::unique_ptr<AttachmentInfo>, Vec4c>> att_infos;
				std::vector<std::unique_ptr<SubpassInfo>> sp_infos;
				for (auto& p : info.passes)
				{
					auto find_or_add_att = [&](void* p) {
						const auto& t = *(RenderpathPassTarget*)p;

						for (auto i = 0; i < att_infos.size(); i++)
						{
							auto& att = att_infos[i];
							if (t.type == std::get<0>(att) && t.v == std::get<1>(att))
								return i;
						}

						auto idx = (int)att_infos.size();

						Image* image = nullptr;
						switch (t.type)
						{
						case RenderpathPassTargetImage:
							image = (Image*)t.v;
							break;
						case RenderpathPassTargetImageview:
							image = ((Imageview*)t.v)->image();
							break;
						case RenderpathPassTargetImages:
							image = (*(std::vector<Image*>*)t.v)[0];
							break;
						}
						assert(image);

						auto att_info = new AttachmentInfo;
						att_info->format = image->format;
						att_info->clear = t.clear;
						att_info->sample_count = image->sample_count;
						att_infos.emplace_back(t.type, t.v, att_info, t.clear_color);

						rp_info.attachments.push_back(att_info);

						return idx;
					};

					auto sp_info = new SubpassInfo;

					for (auto& t : p.color_targets)
						sp_info->color_attachments.push_back(find_or_add_att(t));
					for (auto& t : p.resolve_targets)
						sp_info->resolve_attachments.push_back(find_or_add_att(t));
					if (p.depth_target)
						sp_info->depth_attachment = find_or_add_att(p.depth_target);

					sp_infos.emplace_back(sp_info);
					rp_info.subpasses.push_back(sp_info);
				}
				rp = (RenderpassPrivate*)Renderpass::create(d, rp_info);

				cv = (ClearvaluesPrivate*)Clearvalues::create(rp);
				for (auto i = 0; i < att_infos.size(); i++)
					cv->set(i, std::get<3>(att_infos[i]));

				image_count = 0;
				for (auto& att_info : att_infos)
				{
					auto type = std::get<0>(att_info);
					if (type == RenderpathPassTargetImages)
					{
						auto count = ((std::vector<Image*>*)std::get<1>(att_info))->size();
						if (image_count == 0)
							image_count = count;
						else
							assert(image_count == count);
					}
				}

				for (auto i = 0; i < image_count; i++)
				{
					FramebufferInfo fb_info;
					fb_info.rp = rp;
					std::vector<Imageview*> new_views;
					for (auto& att_info : att_infos)
					{
						auto type = std::get<0>(att_info);
						auto v = std::get<1>(att_info);
						switch (type)
						{
						case RenderpathPassTargetImage:
						{
							auto view = Imageview::create((Image*)v);
							new_views.push_back(view);
							fb_info.views.push_back(view);
						}
							break;
						case RenderpathPassTargetImageview:
							fb_info.views.push_back(v);
							break;
						case RenderpathPassTargetImages:
						{
							auto view = Imageview::create((*(std::vector<Image*>*)v)[i]);
							new_views.push_back(view);
							fb_info.views.push_back(view);
						}
							break;
						}
					}
					auto fb = (FramebufferPrivate*)Framebuffer::create(d, fb_info);
					fbs.emplace_back(fb, new_views);
				}

				for (auto i = 0; i < image_count; i++)
				{
					auto cb = (CommandbufferPrivate*)Commandbuffer::create(d->gcp);
					cb->begin();
					cb->begin_renderpass(rp, fbs[i].first, cv);
					cb->end_renderpass();
					cb->end();
					cbs.push_back(cb);
				}
			}
		};

		uint Renderpath::image_count() const
		{
			return ((RenderpathPrivate*)this)->image_count;
		}

		Renderpass* Renderpath::renderpass() const
		{
			return ((RenderpathPrivate*)this)->rp;
		}

		Framebuffer* Renderpath::framebuffer(uint idx) const
		{
			return ((RenderpathPrivate*)this)->fbs[idx].first;
		}

		Commandbuffer* Renderpath::commandbuffer(uint idx) const
		{
			return ((RenderpathPrivate*)this)->cbs[idx];
		}

		Renderpath* Renderpath::create(Device* d, const RenderpathInfo& info)
		{
			return new RenderpathPrivate(d, info);
		}

		void Renderpath::destroy(Renderpath* s)
		{
			delete (RenderpathPrivate*)s;
		}
	}
}
