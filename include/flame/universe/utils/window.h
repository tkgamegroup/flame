#pragma once

#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/list.h>
#include <flame/universe/components/window.h>
#include <flame/universe/utils/splitter.h>

namespace flame
{
	namespace utils
	{
		inline void make_docker_floating_container(Entity* e, const Vec2f& pos, const Vec2f& size)
		{
			e->set_name("docker_floating_container");
			auto ce = cElement::create();
			ce->pos_ = pos;
			ce->size_ = size + Vec2f(16.f, 28.f + style_1u(FontSize));
			ce->padding_ = Vec4f(8.f, 16.f, 8.f, 8.f);
			ce->frame_thickness_ = 2.f;
			ce->color_ = style_4c(BackgroundColor);
			ce->frame_color_ = style_4c(ForegroundColor);
			e->add_component(ce);
			e->add_component(cEventReceiver::create());
			e->add_component(cLayout::create(LayoutFree));
			e->add_component(cMoveable::create());
			{
				auto e_btf = Entity::create();
				e->add_child(e_btf);
				e_btf->add_component(cElement::create());
				auto cer = cEventReceiver::create();
				cer->pass_checkers.add([](void* c, cEventReceiver* er, bool* pass) {
					*pass = true;
					return true;
				}, Mail());
				e_btf->add_component(cer);
				auto ca = cAligner::create();
				ca->width_policy_ = SizeFitParent;
				ca->height_policy_ = SizeFitParent;
				e_btf->add_component(ca);
				e_btf->add_component(cBringToFront::create());
			}
			{
				auto e_sd = Entity::create();
				e->add_child(e_sd);
				auto ce = cElement::create();
				ce->size_ = 8.f;
				ce->color_ = style_4c(FrameColorHovering);
				e_sd->add_component(ce);
				e_sd->add_component(cEventReceiver::create());
				auto ca = cAligner::create();
				ca->x_align_ = AlignxRight;
				ca->y_align_ = AlignyBottom;
				e_sd->add_component(ca);
				e_sd->add_component(cSizeDragger::create());
			}
		}

		inline void make_docker_layout(Entity* e, LayoutType type)
		{
			e->set_name("docker_layout");
			e->add_component(cElement::create());
			auto ca = cAligner::create();
			ca->x_align_ = AlignxLeft;
			ca->y_align_ = AlignyTop;
			ca->width_policy_ = SizeFitParent;
			ca->height_policy_ = SizeFitParent;
			ca->using_padding_ = true;
			e->add_component(ca);
			auto cl = cLayout::create(type);
			cl->width_fit_children = false;
			cl->height_fit_children = false;
			e->add_component(cl);
			{
				auto es = Entity::create();
				utils::make_splitter(es, type == LayoutHorizontal ? SplitterHorizontal : SplitterVertical);
				e->add_child(es);
			}
		}

		inline void make_docker(Entity* e)
		{
			e->set_name("docker");
			e->add_component(cElement::create());
			auto ca = cAligner::create();
			ca->x_align_ = AlignxLeft;
			ca->y_align_ = AlignyTop;
			ca->width_policy_ = SizeFitParent;
			ca->height_policy_ = SizeFitParent;
			ca->using_padding_ = true;
			e->add_component(ca);
			auto cl = cLayout::create(LayoutVertical);
			cl->width_fit_children = false;
			cl->height_fit_children = false;
			e->add_component(cl);
			{
				auto et = Entity::create();
				et->set_name("docker_tabbar");
				e->add_child(et);
				auto ce = cElement::create();
				ce->clip_flags = ClipChildren;
				et->add_component(ce);
				et->add_component(cEventReceiver::create());
				auto ca = cAligner::create();
				ca->width_policy_ = SizeFitParent;
				et->add_component(ca);
				et->add_component(cLayout::create(LayoutHorizontal));
				et->add_component(cList::create(false));
				et->add_component(cDockerTabbar::create());
			}
			{
				auto ep = Entity::create();
				ep->set_name("docker_pages");
				e->add_child(ep);
				ep->add_component(cElement::create());
				ep->add_component(cEventReceiver::create());
				auto ca = cAligner::create();
				ca->width_policy_ = SizeFitParent;
				ca->height_policy_ = SizeFitParent;
				ep->add_component(ca);
				ep->add_component(cLayout::create(LayoutFree));
				ep->add_component(cDockerPages::create());
			}
		}
	}
}
