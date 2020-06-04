#include <flame/serialize.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include "event_receiver_private.h"
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/list.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/splitter.h>
#include <flame/universe/components/window.h>
#include <flame/universe/components/extra_element_drawing.h>
#include <flame/universe/utils/layer.h>

namespace flame
{
	struct cMoveablePrivate : cMoveable
	{
		void* mouse_listener;

		cMoveablePrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			mouse_listener = nullptr;
		}

		~cMoveablePrivate()
		{
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void on_event(Entity::Event e, void* t) override
		{
			switch (e)
			{
			case Entity::EventComponentAdded:
				if (t == this)
				{
					event_receiver = entity->get_component(cEventReceiver);
					assert(event_receiver);

					mouse_listener = event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						auto thiz = c.thiz<cMoveablePrivate>();
						if (thiz->event_receiver->is_active() && is_mouse_move(action, key))
							thiz->element->add_pos((Vec2f)pos / thiz->element->global_scale, thiz);
						return true;
					}, Capture().set_thiz(this));
				}
				break;
			}
		}
	};

	cMoveable* cMoveable::create()
	{
		return new cMoveablePrivate;
	}

	struct cBringToFrontPrivate : cBringToFront
	{
		void* pass_listener;
		void* mouse_listener;

		cBringToFrontPrivate()
		{
			event_receiver = nullptr;

			pass_listener = nullptr;
			mouse_listener = nullptr;
		}

		~cBringToFrontPrivate()
		{
			if (!entity->dying_)
			{
				event_receiver->pass_checkers.remove(pass_listener);
				event_receiver->mouse_listeners.remove(mouse_listener);
			}
		}

		void on_event(Entity::Event e, void* t) override
		{
			switch (e)
			{
			case Entity::EventComponentAdded:
				if (t == this)
				{
					event_receiver = entity->get_component(cEventReceiver);
					assert(event_receiver);

					pass_listener = event_receiver->pass_checkers.add([](Capture& c, cEventReceiver* er, bool* pass) {
						*pass = true;
						return true;
					}, Capture());
					mouse_listener = event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						if (is_mouse_down(action, key, true) && key == Mouse_Left)
						{
							auto thiz = c.thiz<cBringToFrontPrivate>();
							auto l = thiz->entity->parent->parent->last_child();
							if (!l || !SUS::starts_with(l->name.v, "layer_"))
							{
								looper().add_event([](Capture& c) {
									auto p = c.thiz<Entity>()->parent;
									p->parent->reposition_child(p, -1);
								}, Capture().set_thiz(thiz->entity));
							}
						}
						return true;
					}, Capture().set_thiz(this));
				}
				break;
			}
		}
	};

	cBringToFront* cBringToFront::create()
	{
		return new cBringToFrontPrivate;
	}

	void cBringToFront::make(Entity* e)
	{
		e->add_component(cElement::create());
		e->add_component(cEventReceiver::create());
		auto ca = cAligner::create();
		ca->x_align_flags = AlignFlag(AlignMinMax | AlignAbsolute);
		ca->y_align_flags = AlignFlag(AlignMinMax | AlignAbsolute);
		e->add_component(ca);
		e->add_component(cBringToFront::create());
	}

	struct cSizeDraggerPrivate : cSizeDragger
	{
		void* mouse_listener;
		void* state_listener;

		cSizeDraggerPrivate()
		{
			event_receiver = nullptr;
			p_element = nullptr;

			mouse_listener = nullptr;
			state_listener = nullptr;
		}

		~cSizeDraggerPrivate()
		{
			if (!entity->dying_)
			{
				event_receiver->mouse_listeners.remove(mouse_listener);
				event_receiver->state_listeners.remove(state_listener);
			}
		}

		void on_event(Entity::Event e, void* t) override
		{
			switch (e)
			{
			case Entity::EventComponentAdded:
				if (t == this)
				{
					event_receiver = entity->get_component(cEventReceiver);
					assert(event_receiver);

					mouse_listener = event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						auto thiz = c.thiz<cSizeDraggerPrivate>();
						if (is_mouse_move(action, key) && thiz->event_receiver->is_active())
							thiz->p_element->add_size(Vec2f(pos));
						return true;
					}, Capture().set_thiz(this));
					state_listener = event_receiver->state_listeners.add([](Capture& c, EventReceiverState s) {
						c.current<cEventReceiver>()->dispatcher->window->set_cursor(s ? CursorSizeNWSE : CursorArrow);
						return true;
					}, Capture().set_thiz(this));

					auto p = entity->parent;
					if (p)
						p_element = p->get_component(cElement);
				}
				break;
			}
		}
	};

	cSizeDragger* cSizeDragger::create()
	{
		return new cSizeDraggerPrivate;
	}

	void cSizeDragger::make(World* w, Entity* e)
	{
		auto sg = (StyleGetter*)w->find_object(FLAME_CHASH("StyleGetter"), 0);
		auto ce = cElement::create();
		ce->size = 8.f;
		e->add_component(ce);
		auto ceed = cExtraElementDrawing::create();
		ceed->color = sg->get_style(FrameColorHovering).c;
		ceed->draw_flags = ExtraDrawFilledCornerSE;
		e->add_component(ceed);
		e->add_component(cEventReceiver::create());
		auto ca = cAligner::create();
		ca->x_align_flags = AlignFlag(AlignMax | AlignAbsolute);
		ca->y_align_flags = AlignFlag(AlignMax | AlignAbsolute);
		e->add_component(ca);
		e->add_component(cSizeDragger::create());
	}

	struct cDockerTabPrivate : cDockerTab
	{
		void* mouse_listener;
		void* drag_and_drop_listener;
		Vec2f drop_pos;
		void* draw_cmd;

		struct DropTip
		{
			bool highlighted;
			Vec2f pos;
			Vec2f size;
		};
		std::vector<DropTip> drop_tips;
		cEventReceiver* overing;

		cDockerTabPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;
			list_item = nullptr;

			root = nullptr;

			floating = false;
			page = nullptr;
			page_element = nullptr;

			mouse_listener = nullptr;
			drag_and_drop_listener = nullptr;
			drop_pos = Vec2f(0.f);
			draw_cmd = nullptr;

			overing = nullptr;
		}

		~cDockerTabPrivate()
		{
			if (!entity->dying_)
			{
				element->cmds.remove(draw_cmd);
				event_receiver->mouse_listeners.remove(mouse_listener);
				event_receiver->drag_and_drop_listeners.remove(drag_and_drop_listener);
			}
		}

		void take_away(bool close)
		{
			auto tabbar = entity->parent;
			if (tabbar->name.h != FLAME_CHASH("docker_tabbar"))
				return;

			auto docker = tabbar->parent;
			auto pages = docker->children[1];
			page = pages->children[entity->index_];
			page_element = page->get_component(cElement);
			auto page_aligner = page->get_component(cAligner);
			auto list = list_item->list;
			list_item->list = nullptr;

			if (tabbar->children.s > 1 && list && list->selected == entity)
				list->set_selected(list->entity->children[0]);

			if (close)
			{
				pages->remove_child(page); // remove tab first will lead variable 'page' to be trash data
				tabbar->remove_child(entity);
			}
			else
			{
				auto dp = event_receiver->dispatcher;

				tabbar->remove_child(entity, false);
				pages->remove_child(page, false);
				page->set_visible(true);
				element->set_pos(element->global_pos);
				element->set_alpha(0.5f);
				page_element->set_pos(Vec2f(element->pos.x(), element->pos.y() + element->global_size.y() + 8.f));
				page_element->set_alpha(0.5f);
				page_aligner->set_x_align_flags(0);
				page_aligner->set_y_align_flags(0);
				root->add_child(page);
				root->add_child(entity);

				dp->focusing = event_receiver;
				dp->focusing_state = FocusingAndDragging;
			}

			if (tabbar->children.s == 0)
			{
				auto p = docker->parent;
				if (p)
				{
					if (p->name.h == FLAME_CHASH("docker_floating_container"))
						p->parent->remove_child(p);
					else if (p->name.h == FLAME_CHASH("docker_static_container"))
						p->remove_children(0, -1);
					else if (p->name.h == FLAME_CHASH("docker_layout"))
					{
						auto oth_docker = p->children[docker->index_ == 0 ? 2 : 0];
						p->remove_child(oth_docker, false);
						auto pp = p->parent;
						auto idx = p->index_;
						pp->remove_child(p);
						pp->add_child(oth_docker, idx);
					}
				}
			}
		}

		void on_event(Entity::Event e, void* t) override
		{
			switch (e)
			{
			case Entity::EventComponentAdded:
				if (t == this)
				{
					element = entity->get_component(cElement);
					event_receiver = entity->get_component(cEventReceiver);
					list_item = entity->get_component(cListItem);
					assert(element);
					assert(event_receiver);
					assert(list_item);

					draw_cmd = element->cmds.add([](Capture& c, graphics::Canvas* canvas) {
						c.thiz<cDockerTabPrivate>()->draw(canvas);
						return true;
					}, Capture().set_thiz(this));

					event_receiver->drag_hash = FLAME_CHASH("cDockerTab");
					mouse_listener = event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						auto thiz = c.thiz<cDockerTabPrivate>();
						if (is_mouse_move(action, key) && thiz->event_receiver->is_dragging() && thiz->page)
						{
							thiz->element->add_pos(Vec2f(pos));
							thiz->page_element->add_pos(Vec2f(pos));
						}
						return true;
					}, Capture().set_thiz(this));
					drag_and_drop_listener = event_receiver->drag_and_drop_listeners.add([](Capture& c, DragAndDrop action, cEventReceiver* er, const Vec2i& pos) {
						auto thiz = c.thiz<cDockerTabPrivate>();
						if (action == DragStart)
						{
							thiz->floating = true;
							looper().add_event([](Capture& c) {
								c.thiz<cDockerTabPrivate>()->take_away(false);
							}, Capture().set_thiz(thiz));
						}
						else if (action == DragEnd)
						{
							if (!er || thiz->floating)
							{
								thiz->drop_pos = pos;
								looper().add_event([](Capture& c) {
									auto thiz = c.thiz<cDockerTabPrivate>();
									auto e_tab = thiz->entity;
									auto e_page = thiz->page;
									auto page_element = thiz->page_element;
									auto page_aligner = e_page->get_component(cAligner);
									auto w = thiz->root->world;

									auto e_container = f_new<Entity>();
									cDockerTab::make_floating_container(w, e_container, thiz->drop_pos, page_element->size);
									thiz->root->add_child(e_container);

									auto e_docker = f_new<Entity>();
									cDockerTab::make_docker(w, e_docker);
									e_container->add_child(e_docker, 0);

									auto e_tabbar = e_docker->children[0];
									auto e_pages = e_docker->children[1];

									thiz->root->remove_child(e_tab, false);
									thiz->root->remove_child(e_page, false);
									thiz->list_item->list = e_tabbar->get_component(cList);
									e_tabbar->add_child(e_tab);
									auto element = thiz->element;
									element->set_pos(Vec2f(0.f));
									element->set_alpha(1.f);
									e_pages->add_child(e_page);
									page_element->set_pos(Vec2f(0.f));
									page_element->set_alpha(1.f);
									page_aligner->set_x_align_flags(AlignMinMax);
									page_aligner->set_y_align_flags(AlignMinMax);
									thiz->page = nullptr;
									thiz->page_element = nullptr;
								}, Capture().set_thiz(thiz));
							}
						}
						return true;
					}, Capture().set_thiz(this));

					auto p = entity->parent;
					if (p && p->children.s == 1)
						p->get_component(cDockerTabbar)->list->set_selected(entity);
				}
				break;
			}
		}

		void draw(graphics::Canvas* canvas)
		{
			if (!overing || event_receiver->dispatcher->drag_overing != overing)
			{
				drop_tips.clear();
				overing = nullptr;
			}
			else
			{
				if (!element->clipped)
				{
					for (auto p : drop_tips)
					{
						std::vector<Vec2f> points;
						path_rect(points, p.pos, p.size);
						canvas->fill(points.size(), points.data(), p.highlighted ? Vec4c(60, 90, 210, 255) : Vec4c(50, 80, 200, 200));
					}
				}
			}
		}
	};

	void cDockerTab::take_away(bool close)
	{
		((cDockerTabPrivate*)this)->take_away(close);
	}

	cDockerTab* cDockerTab::create()
	{
		return new cDockerTabPrivate;
	}

	void cDockerTab::make_floating_container(World* w, Entity* e, const Vec2f& pos, const Vec2f& size)
	{
		auto sg = (StyleGetter*)w->find_object(FLAME_CHASH("StyleGetter"), 0);
		e->name = "docker_floating_container";
		auto ce = cElement::create();
		ce->pos = pos;
		ce->size = size + Vec2f(16.f, 28.f + sg->get_style(FontSize).u[0]);
		ce->padding = Vec4f(8.f, 16.f, 8.f, 8.f);
		ce->frame_thickness = 2.f;
		ce->color = sg->get_style(BackgroundColor).c;
		ce->frame_color = sg->get_style(ForegroundColor).c;
		e->add_component(ce);
		e->add_component(cEventReceiver::create());
		e->add_component(cLayout::create(LayoutFree));
		e->add_component(cMoveable::create());
		auto e_btf = f_new<Entity>();
		cBringToFront::make(e_btf);
		e->add_child(e_btf);
		auto e_sd = f_new<Entity>();
		cSizeDragger::make(w, e_sd);
		e->add_child(e_sd);
	}

	void cDockerTab::make_static_container(World* w, Entity* e)
	{
		auto sg = (StyleGetter*)w->find_object(FLAME_CHASH("StyleGetter"), 0);
		e->name = "docker_static_container";
		auto ce = cElement::create();
		ce->color = sg->get_style(BackgroundColor).c;
		e->add_component(ce);
		e->add_component(cEventReceiver::create());
		auto ca = cAligner::create();
		ca->x_align_flags = AlignMinMax;
		ca->y_align_flags = AlignMinMax;
		e->add_component(ca);
		e->add_component(cLayout::create(LayoutFree));
		e->add_component(cDockerStaticContainer::create());
	}

	void cDockerTab::make_layout(World* w, Entity* e, LayoutType type)
	{
		e->name = "docker_layout";
		e->add_component(cElement::create());
		auto ca = cAligner::create();
		ca->x_align_flags = AlignMinMax;
		ca->y_align_flags = AlignMinMax;
		e->add_component(ca);
		auto cl = cLayout::create(type);
		cl->width_fit_children = false;
		cl->height_fit_children = false;
		e->add_component(cl);
		auto es = f_new<Entity>();
		cSplitter::make(w, es, type == LayoutHorizontal ? SplitterHorizontal : SplitterVertical);
		e->add_child(es);
	}

	void cDockerTab::make_docker(World* w, Entity* e)
	{
		auto sg = (StyleGetter*)w->find_object(FLAME_CHASH("StyleGetter"), 0);
		e->name = "docker";
		auto ce = cElement::create();
		ce->frame_thickness = 1.f;
		ce->frame_color = sg->get_style(ForegroundColor).c;
		e->add_component(ce);
		auto ca = cAligner::create();
		ca->x_align_flags = AlignMinMax;
		ca->y_align_flags = AlignMinMax;
		e->add_component(ca);
		auto cl = cLayout::create(LayoutVertical);
		cl->width_fit_children = false;
		cl->height_fit_children = false;
		e->add_component(cl);
		{
			auto et = f_new<Entity>();
			et->name = "docker_tabbar";
			e->add_child(et);
			auto ce = cElement::create();
			ce->clip_flags = ClipChildren;
			et->add_component(ce);
			et->add_component(cEventReceiver::create());
			auto ca = cAligner::create();
			ca->x_align_flags = AlignMinMax;
			et->add_component(ca);
			et->add_component(cLayout::create(LayoutHorizontal));
			et->add_component(cList::create(false));
			et->add_component(cDockerTabbar::create());
		}
		{
			auto ep = f_new<Entity>();
			ep->name = "docker_pages";
			e->add_child(ep);
			auto ce = cElement::create();
			ce->padding = 2.f;
			ep->add_component(ce);
			ep->add_component(cEventReceiver::create());
			auto ca = cAligner::create();
			ca->x_align_flags = AlignMinMax;
			ca->y_align_flags = AlignMinMax;
			ep->add_component(ca);
			ep->add_component(cLayout::create(LayoutFree));
			ep->add_component(cDockerPages::create());
		}
	}

	struct cDockerTabbarPrivate : cDockerTabbar
	{
		void* drag_and_drop_listener;
		cDockerTab* drop_tab;
		uint drop_idx;
		void* selected_changed_listener;

		cDockerTabbarPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;
			list = nullptr;

			drag_and_drop_listener = nullptr;
			drop_tab = nullptr;
			drop_idx = 0;
		}

		~cDockerTabbarPrivate()
		{
			if (!entity->dying_)
			{
				event_receiver->drag_and_drop_listeners.remove(drag_and_drop_listener);
				list->data_changed_listeners.remove(selected_changed_listener);
			}
		}

		uint calc_pos(float x, float* out)
		{
			for (auto i = 0; i < entity->children.s; i++)
			{
				auto element = entity->children[i]->get_component(cElement);
				auto half = element->global_pos.x() + element->size.x() * 0.5f;
				if (x >= element->global_pos.x() && x < half)
				{
					if (out)
						*out = element->global_pos.x();
					return i;
				}
				if (x >= half && x < element->global_pos.x() + element->size.x())
				{
					if (out)
						*out = element->global_pos.x() + element->size.x();
					return i + 1;
				}
			}
			if (out)
			{
				auto element = entity->children[entity->children.s - 1]->get_component(cElement);
				*out = element->global_pos.x() + element->global_size.x();
			}
			return entity->children.s;
		}

		void on_event(Entity::Event e, void* t) override
		{
			switch (e)
			{
			case Entity::EventComponentAdded:
				if (t == this)
				{
					element = entity->get_component(cElement);
					event_receiver = entity->get_component(cEventReceiver);
					list = entity->get_component(cList);
					assert(element);
					assert(event_receiver);
					assert(list);

					event_receiver->set_acceptable_drops(1, &FLAME_CHASH("cDockerTab"));
					drag_and_drop_listener = event_receiver->drag_and_drop_listeners.add([](Capture& c, DragAndDrop action, cEventReceiver* er, const Vec2i& pos) {
						auto thiz = c.thiz<cDockerTabbarPrivate>();
						if (thiz->entity->children.s > 0) // a valid docker tabbar must have at least one item
						{
							if (action == BeingOvering)
							{
								float show_drop_pos;
								auto idx = thiz->calc_pos(pos.x(), &show_drop_pos);
								if (idx == thiz->entity->children.s)
									show_drop_pos -= 10.f;
								else if (idx != 0)
									show_drop_pos -= 5.f;
								auto drop_tab = (cDockerTabPrivate*)er->entity->get_component(cDockerTab);
								drop_tab->drop_tips.clear();
								drop_tab->overing = thiz->event_receiver;
								cDockerTabPrivate::DropTip primitive;
								primitive.highlighted = false;
								primitive.pos = Vec2f(show_drop_pos, thiz->element->global_pos.y());
								primitive.size = Vec2f(10.f, thiz->element->global_size.y());
								drop_tab->drop_tips.push_back(primitive);
							}
							else if (action == BeenDropped)
							{
								thiz->drop_tab = er->entity->get_component(cDockerTab);
								thiz->drop_idx = thiz->calc_pos(pos.x(), nullptr);
								thiz->drop_tab->floating = false;
								looper().add_event([](Capture& c) {
									auto thiz = c.thiz<cDockerTabbarPrivate>();
									auto tabbar = thiz->entity;
									auto tab = thiz->drop_tab;
									auto e_tab = tab->entity;
									auto e_page = tab->page;
									auto page_element = tab->page_element;
									auto page_aligner = e_page->get_component(cAligner);

									tab->root->remove_child(e_tab, false);
									tab->root->remove_child(e_page, false);
									tab->list_item->list = thiz->list;
									tabbar->add_child(e_tab, thiz->drop_idx);
									tabbar->parent->children[1]->add_child(e_page);

									tab->element->set_alpha(1.f);
									page_element->set_pos(Vec2f(0.f));
									page_element->set_alpha(1.f);
									page_aligner->set_x_align_flags(AlignMinMax);
									page_aligner->set_y_align_flags(AlignMinMax);

									thiz->drop_tab->page = nullptr;
									thiz->drop_tab->page_element = nullptr;
									thiz->drop_tab = nullptr;
									thiz->drop_idx = 0;

									thiz->list->set_selected(e_tab);
								}, Capture().set_thiz(thiz));
							}
						}
						return true;
					}, Capture().set_thiz(this));

					selected_changed_listener = list->data_changed_listeners.add([](Capture& c, uint hash, void*) {
						auto thiz = c.thiz<cDockerTabbarPrivate>();
						if (hash == FLAME_CHASH("selected"))
						{
							auto tabbar = thiz->entity;
							auto docker = tabbar->parent;
							auto pages = docker->children[1];
							if (pages->children.s > 0)
							{
								auto idx = thiz->list->selected->index_;
								for (auto i : pages->children)
									i->set_visible(false);
								pages->children[idx]->set_visible(true);
							}
						}
						return true;
					}, Capture().set_thiz(this));
				}
				break;
			}
		}
	};

	cDockerTabbar* cDockerTabbar::create()
	{
		return new cDockerTabbarPrivate;
	}

	struct cDockerPagesPrivate : cDockerPages 
	{
		void* drag_and_drop_listener;
		cDockerTab* drop_tab;
		Side dock_side;

		cDockerPagesPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			drag_and_drop_listener = nullptr;
			drop_tab = nullptr;
			dock_side = Outside;
		}

		~cDockerPagesPrivate()
		{
			if (!entity->dying_)
				event_receiver->drag_and_drop_listeners.remove(drag_and_drop_listener);
		}

		void on_event(Entity::Event e, void* t) override
		{
			switch (e)
			{
			case Entity::EventComponentAdded:
				if (t == this)
				{
					element = entity->get_component(cElement);
					event_receiver = entity->get_component(cEventReceiver);
					assert(element);
					assert(event_receiver);

					event_receiver->set_acceptable_drops(1, &FLAME_CHASH("cDockerTab"));
					drag_and_drop_listener = event_receiver->drag_and_drop_listeners.add([](Capture& c, DragAndDrop action, cEventReceiver* er, const Vec2i& pos) {
						auto thiz = c.thiz<cDockerPagesPrivate>();
						if (action == BeingOvering)
						{
							thiz->dock_side = Outside;
							auto center = thiz->element->center();
							if (rect_contains(Vec4f(center + Vec2f(-25.f, -25.f), center + Vec2f(25.f, 25.f)), (Vec2f)pos))
								thiz->dock_side = SideCenter;
							else if (rect_contains(Vec4f(center + Vec2f(-55.f, -25.f), center + Vec2f(-30.f, 25.f)), (Vec2f)pos))
								thiz->dock_side = SideW;
							else if (rect_contains(Vec4f(center + Vec2f(30.f, -25.f), center + Vec2f(55.f, 25.f)), (Vec2f)pos))
								thiz->dock_side = SideE;
							else if (rect_contains(Vec4f(center + Vec2f(-25.f, -55.f), center + Vec2f(25.f, -30.f)), (Vec2f)pos))
								thiz->dock_side = SideN;
							else if (rect_contains(Vec4f(center + Vec2f(-25.f, 30.f), center + Vec2f(25.f, 55.f)), (Vec2f)pos))
								thiz->dock_side = SideS;

							auto drop_tab = (cDockerTabPrivate*)er->entity->get_component(cDockerTab);
							drop_tab->drop_tips.clear();
							drop_tab->overing = thiz->event_receiver;
							{
								cDockerTabPrivate::DropTip primitive;
								primitive.highlighted = thiz->dock_side == SideCenter;
								primitive.pos = center + Vec2f(-25.f);
								primitive.size = Vec2f(50.f, 50.f);
								std::vector<Vec2f> points;
								drop_tab->drop_tips.push_back(primitive);
							}
							{
								cDockerTabPrivate::DropTip primitive;
								primitive.highlighted = thiz->dock_side == SideW;
								primitive.pos = center + Vec2f(-55.f, -25.f);
								primitive.size = Vec2f(25.f, 50.f);
								std::vector<Vec2f> points;
								drop_tab->drop_tips.push_back(primitive);
							}
							{
								cDockerTabPrivate::DropTip primitive;
								primitive.highlighted = thiz->dock_side == SideE;
								primitive.pos = center + Vec2f(30.f, -25.f);
								primitive.size = Vec2f(25.f, 50.f);
								std::vector<Vec2f> points;
								drop_tab->drop_tips.push_back(primitive);
							}
							{
								cDockerTabPrivate::DropTip primitive;
								primitive.highlighted = thiz->dock_side == SideN;
								primitive.pos = center + Vec2f(-25.f, -55.f);
								primitive.size = Vec2f(50.f, 25.f);
								std::vector<Vec2f> points;
								drop_tab->drop_tips.push_back(primitive);
							}
							{
								cDockerTabPrivate::DropTip primitive;
								primitive.highlighted = thiz->dock_side == SideS;
								primitive.pos = center + Vec2f(-25.f, 30.f);
								primitive.size = Vec2f(50.f, 25.f);
								std::vector<Vec2f> points;
								drop_tab->drop_tips.push_back(primitive);
							}
						}
						else if (action == BeenDropped)
						{
							if (thiz->dock_side == SideCenter)
							{
								auto tabbar_er = (cEventReceiverPrivate*)thiz->entity->parent->children[0]->get_component(cEventReceiver);
								tabbar_er->on_drag_and_drop(BeenDropped, er, Vec2i(0, 99999));
							}
							else if (thiz->dock_side != Outside)
							{
								thiz->drop_tab = er->entity->get_component(cDockerTab);
								thiz->drop_tab->floating = false;
								looper().add_event([](Capture& c) {
									auto thiz = c.thiz<cDockerPagesPrivate>();
									auto tab = thiz->drop_tab;
									auto e_tab = tab->entity;
									auto e_page = tab->page;
									auto page_element = tab->page_element;
									auto page_aligner = e_page->get_component(cAligner);
									auto docker = thiz->entity->parent;
									auto docker_element = docker->get_component(cElement);
									auto docker_aligner = docker->get_component(cAligner);
									auto p = docker->parent;
									auto docker_idx = docker->index_; LayoutFree;
									auto w = thiz->entity->world;

									auto layout = f_new<Entity>();
									p->remove_child(docker, false);
									cDockerTab::make_layout(w, layout, (thiz->dock_side == SideW || thiz->dock_side == SideE) ? LayoutHorizontal : LayoutVertical);
									p->add_child(layout, docker_idx);

									if (p->name.h != FLAME_CHASH("docker_floating_container") && p->name.h != FLAME_CHASH("docker_static_container"))
									{
										auto p_element = p->get_component(cElement);
										auto layout_element = layout->get_component(cElement);

										layout_element->set_size(p_element->size);

										auto aligner = layout->get_component(cAligner);
										aligner->set_width_factor(p_element->size.x());
										aligner->set_height_factor(p_element->size.y());

										{
											auto oth = p->children[docker_idx == 0 ? 2 : 0];
											auto element = oth->get_component(cElement);
											auto aligner = oth->get_component(cAligner);
											aligner->set_width_factor(element->size.x());
											aligner->set_height_factor(element->size.y());
										}
									}

									auto new_docker = f_new<Entity>();
									cDockerTab::make_docker(w, new_docker);
									switch (thiz->dock_side)
									{
									case SideW:
									case SideN:
										layout->add_child(new_docker, 0);
										layout->add_child(docker, 2);
										break;
									case SideE:
									case SideS:
										layout->add_child(docker, 0);
										layout->add_child(new_docker, 2);
										break;
									}
									auto new_docker_element = new_docker->get_component(cElement);
									auto new_docker_aligner = new_docker->get_component(cAligner);
									auto new_tabbar = new_docker->children[0];
									auto new_pages = new_docker->children[1];

									tab->root->remove_child(e_tab, false);
									tab->root->remove_child(e_page, false);
									tab->list_item->list = (cList*)new_tabbar->get_component(cList);
									new_tabbar->add_child(e_tab);
									new_pages->add_child(e_page);

									tab->element->set_alpha(1.f);
									page_element->set_pos(Vec2f(0.f));
									page_element->set_alpha(1.f);
									page_aligner->set_x_align_flags(AlignMinMax);
									page_aligner->set_y_align_flags(AlignMinMax);

									auto e_splitter = layout->children[1];
									auto splitter_element = e_splitter->get_component(cElement);
									auto splitter = e_splitter->get_component(cSplitter);
									if (thiz->dock_side == SideW || thiz->dock_side == SideE)
									{
										layout->get_component(cLayout)->type = LayoutHorizontal;
										splitter->type = SplitterHorizontal;
										e_splitter->get_component(cAligner)->set_y_align_flags(AlignMinMax);

										auto w = (docker_element->size.x() - splitter_element->size.x()) * 0.5f;
										docker_element->set_width(w);
										new_docker_element->set_width(w);
										docker_aligner->set_width_factor(w);
										new_docker_aligner->set_width_factor(w);

										auto h = docker_element->size.y() * 0.5f;
										docker_element->set_height(h);
										new_docker_element->set_height(h);
										docker_aligner->set_height_factor(h);
										new_docker_aligner->set_height_factor(h);
									}
									else
									{
										layout->get_component(cLayout)->type = LayoutVertical;
										splitter->type = SplitterVertical;
										e_splitter->get_component(cAligner)->set_x_align_flags(AlignMinMax);

										auto w = docker_element->size.x();
										docker_element->set_width(w);
										new_docker_element->set_width(w);
										docker_aligner->set_width_factor(w);
										new_docker_aligner->set_width_factor(w);

										auto h = (docker_element->size.y() - splitter_element->size.y()) * 0.5f;
										docker_element->set_height(h);
										new_docker_element->set_height(h);
										docker_aligner->set_height_factor(h);
										new_docker_aligner->set_height_factor(h);
									}

								}, Capture().set_thiz(thiz));
							}
						}
						return true;
					}, Capture().set_thiz(this));
				}
				break;
			}
		}
	};

	cDockerPages* cDockerPages::create()
	{
		return new cDockerPagesPrivate;
	}

	struct cDockerStaticContainerPrivate : cDockerStaticContainer
	{
		void* drag_and_drop_listener;
		cDockerTab* drop_tab;
		Side dock_side;

		cDockerStaticContainerPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			drag_and_drop_listener = nullptr;
			drop_tab = nullptr;

		}

		~cDockerStaticContainerPrivate()
		{
			if (!entity->dying_)
				event_receiver->drag_and_drop_listeners.remove(drag_and_drop_listener);
		}

		void on_event(Entity::Event e, void* t) override
		{
			switch (e)
			{
			case Entity::EventComponentAdded:
				if (t == this)
				{
					element = entity->get_component(cElement);
					event_receiver = entity->get_component(cEventReceiver);
					assert(element);
					assert(event_receiver);

					event_receiver->set_acceptable_drops(1, &FLAME_CHASH("cDockerTab"));
					drag_and_drop_listener = event_receiver->drag_and_drop_listeners.add([](Capture& c, DragAndDrop action, cEventReceiver* er, const Vec2i& pos) {
						auto thiz = c.thiz<cDockerStaticContainerPrivate>();
						if (action == BeingOvering)
						{
							thiz->dock_side = Outside;
							auto center = thiz->element->center();
							if (rect_contains(Vec4f(center + Vec2f(-25.f, -25.f), center + Vec2f(25.f, 25.f)), (Vec2f)pos))
								thiz->dock_side = SideCenter;

							auto drop_tab = (cDockerTabPrivate*)er->entity->get_component(cDockerTab);
							drop_tab->drop_tips.clear();
							drop_tab->overing = thiz->event_receiver;
							{
								cDockerTabPrivate::DropTip primitive;
								primitive.highlighted = thiz->dock_side == SideCenter;
								primitive.pos = center + Vec2f(-25.f);
								primitive.size = Vec2f(50.f, 50.f);
								std::vector<Vec2f> points;
								drop_tab->drop_tips.push_back(primitive);
							}
						}
						else if (action == BeenDropped)
						{
							if (thiz->dock_side == SideCenter)
							{
								thiz->drop_tab = er->entity->get_component(cDockerTab);
								thiz->drop_tab->floating = false;
								looper().add_event([](Capture& c) {
									auto thiz = c.thiz<cDockerStaticContainerPrivate>();
									auto tab = thiz->drop_tab;
									auto e_tab = tab->entity;
									auto e_page = tab->page;
									auto page_element = tab->page_element;
									auto page_aligner = e_page->get_component(cAligner);

									auto new_docker = f_new<Entity>();
									cDockerTab::make_docker(thiz->entity->world, new_docker);
									thiz->entity->add_child(new_docker);
									auto new_tabbar = new_docker->children[0];
									auto new_pages = new_docker->children[1];

									tab->root->remove_child(e_tab, false);
									tab->root->remove_child(e_page, false);
									tab->list_item->list = (cList*)new_tabbar->get_component(cList);
									new_tabbar->add_child(e_tab);
									new_pages->add_child(e_page);

									tab->element->set_alpha(1.f);
									page_element->set_pos(Vec2f(0.f));
									page_element->set_alpha(1.f);
									page_aligner->set_x_align_flags(AlignMinMax);
									page_aligner->set_y_align_flags(AlignMinMax);
								}, Capture().set_thiz(thiz));
							}
						}
						return true;
					}, Capture().set_thiz(this));
				}
				break;
			}
		}
	};

	cDockerStaticContainer* cDockerStaticContainer::create()
	{
		return new cDockerStaticContainerPrivate;
	}
}
