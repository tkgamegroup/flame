#include <flame/graphics/font.h>
#include <flame/universe/topmost.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_dispatcher.h>
#include "event_receiver_private.h"
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/list.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/splitter.h>
#include <flame/universe/components/window.h>

#include "../renderpath/canvas_make_cmd/canvas.h"

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
			if (!entity->dying)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void start() 
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);

			mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				auto thiz = *(cMoveablePrivate**)c;
				if (thiz->event_receiver->active && is_mouse_move(action, key))
				{
					auto e = thiz->element;
					e->pos += (Vec2f)pos / e->global_scale;
				}
			}, new_mail_p(this));
		}
	};

	void cMoveable::start()
	{
		((cMoveablePrivate*)this)->start();
	}

	void cMoveable::update()
	{
	}

	Component* cMoveable::copy()
	{
		return new cMoveablePrivate;
	}

	cMoveable* cMoveable::create()
	{
		return new cMoveablePrivate;
	}
	struct cBringToFrontPrivate : cBringToFront
	{
		void* mouse_listener;

		cBringToFrontPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			mouse_listener = nullptr;
		}

		~cBringToFrontPrivate()
		{
			if (!entity->dying)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);

			mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				auto thiz = *(cBringToFrontPrivate**)c;
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					looper().add_delay_event([](void* c) {
						auto e = *(Entity**)c;
						auto p = e->parent();
						auto pp = p->parent();
						auto idx = pp->child_count() - 1;
						if (idx == 0)
							return;
						if (get_topmost(pp))
							idx -= 1;
						pp->reposition_child(p, idx);
					}, new_mail_p(thiz->entity));
				}
			}, new_mail_p(this));
		}
	};

	void cBringToFront::start()
	{
		((cBringToFrontPrivate*)this)->start();
	}

	void cBringToFront::update()
	{
	}

	Component* cBringToFront::copy()
	{
		return new cBringToFrontPrivate;
	}

	cBringToFront* cBringToFront::create()
	{
		return new cBringToFrontPrivate;
	}

	struct cSizeDraggerPrivate : cSizeDragger
	{
		void* mouse_listener;

		cSizeDraggerPrivate()
		{
			event_receiver = nullptr;
			p_element = nullptr;

			mouse_listener = nullptr;
		}

		~cSizeDraggerPrivate()
		{
			if (!entity->dying)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void start()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			p_element = (cElement*)(entity->parent()->find_component(cH("Element")));
			assert(p_element);

			mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				auto thiz = (*(cSizeDraggerPrivate**)c);
				if (is_mouse_move(action, key) && thiz->event_receiver->active)
					thiz->p_element->size += pos;
			}, new_mail_p(this));
		}
	};

	void cSizeDragger::start()
	{
		((cSizeDraggerPrivate*)this)->start();
	}

	void cSizeDragger::update()
	{
	}

	Component* cSizeDragger::copy()
	{
		return new cSizeDraggerPrivate;
	}

	cSizeDragger* cSizeDragger::create()
	{
		return new cSizeDraggerPrivate;
	}

	struct cDockerTabPrivate : cDockerTab
	{
		void* mouse_listener;
		void* drag_and_drop_listener;
		Vec2f drop_pos;

		struct Primitive
		{
			Vec4c col;
			Vec2f pos;
			Vec2f size;
		};
		std::vector<Primitive> drop_tips;

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
		}

		~cDockerTabPrivate()
		{
			if (!entity->dying)
			{
				event_receiver->mouse_listeners.remove(mouse_listener);
				event_receiver->drag_and_drop_listeners.remove(drag_and_drop_listener);
			}
		}

		void take_away(bool close)
		{
			auto tabbar = entity->parent();
			if (tabbar->name_hash() != cH("docker_tabbar"))
				return;

			auto docker = tabbar->parent();
			auto pages = docker->child(1);
			auto idx = tabbar->child_position(entity);
			page = pages->child(idx);
			page_element = (cElement*)page->find_component(cH("Element"));
			auto page_aligner = (cAligner*)page->find_component(cH("Aligner"));

			if (close)
			{
				pages->remove_child(page); // remove tab first will lead variable 'page' to be trash data
				tabbar->remove_child(entity);
			}
			else
			{
				tabbar->remove_child(entity, false);
				pages->remove_child(page, false);
				page->visible = true;
				element->pos = element->global_pos;
				element->alpha = 0.5f;
				page_element->pos.x() = element->pos.x();
				page_element->pos.y() = element->pos.y() + element->global_size.y() + 8.f;
				page_element->alpha = 0.5f;
				page_aligner->width_policy = SizeFixed;
				page_aligner->height_policy = SizeFixed;
				root->add_child(page);
				root->add_child(entity);
			}

			if (tabbar->child_count() == 0)
			{
				auto p = docker->parent();
				if (p)
				{
					if (p->name_hash() == cH("docker_container"))
						p->parent()->remove_child(p);
					else if (p->name_hash() == cH("docker_layout"))
					{
						auto oth_docker = p->child(p->child_position(docker) == 0 ? 2 : 0);
						p->remove_child(oth_docker, false);
						auto pp = p->parent();
						auto idx = pp->child_position(p);
						pp->remove_child(p);
						pp->add_child(oth_docker, idx);
						if (pp->name_hash() == cH("docker_container"))
						{
							auto aligner = (cAligner*)oth_docker->find_component(cH("Aligner"));
							aligner->x_align = AlignxLeft;
							aligner->y_align = AlignyTop;
							aligner->using_padding = true;
						}
					}
				}
			}
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			event_receiver->drag_hash = cH("DockerTab");
			list_item = (cListItem*)(entity->find_component(cH("ListItem")));
			assert(list_item);
			assert(entity->parent()->name_hash() == cH("docker_tabbar"));

			mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				auto thiz = (*(cDockerTabPrivate**)c);
				if (is_mouse_move(action, key) && thiz->event_receiver->dragging && thiz->page)
				{
					thiz->element->pos += pos;
					thiz->page_element->pos += pos;
				}
			}, new_mail_p(this));

			drag_and_drop_listener = event_receiver->drag_and_drop_listeners.add([](void* c, DragAndDrop action, cEventReceiver* er, const Vec2i& pos) {
				auto thiz = (*(cDockerTabPrivate**)c);
				if (action == DragStart)
				{
					thiz->floating = true;
					auto list = thiz->list_item->list;
					if (list && list->selected == thiz->entity)
						list->set_selected(list->entity->child(0));
					thiz->list_item->list = nullptr;
					looper().add_delay_event([](void* c) {
						auto thiz = *(cDockerTabPrivate**)c;
						thiz->take_away(false);
					}, new_mail_p(thiz));
				}
				else if (action == DragEnd)
				{
					if (!er || thiz->floating)
					{
						thiz->drop_pos = pos;
						looper().add_delay_event([](void* c) {
							auto thiz = (*(cDockerTabPrivate**)c);

							auto e_tab = thiz->entity;
							auto e_page = thiz->page;
							auto page_element = thiz->page_element;
							auto page_aligner = (cAligner*)e_page->find_component(cH("Aligner"));

							auto e_container = get_docker_container_model()->copy();
							thiz->root->add_child(e_container);
							{
								auto c_element = (cElement*)e_container->find_component(cH("Element"));
								c_element->pos = thiz->drop_pos;
								c_element->size = page_element->size;
							}

							auto e_docker = get_docker_model()->copy();
							e_container->add_child(e_docker, 0);
							auto e_tabbar = e_docker->child(0);
							auto e_pages = e_docker->child(1);

							thiz->root->remove_child(e_tab, false);
							thiz->root->remove_child(e_page, false);
							thiz->list_item->list = (cList*)e_tabbar->find_component(cH("List"));
							e_tabbar->add_child(e_tab);
							auto element = thiz->element;
							element->pos = 0.f;
							element->alpha = 1.f;
							e_pages->add_child(e_page);
							page_element->pos = 0.f;
							page_element->alpha = 1.f;
							page_aligner->width_policy = SizeFitParent;
							page_aligner->height_policy = SizeFitParent;
							thiz->page = nullptr;
							thiz->page_element = nullptr;
						}, new_mail_p(thiz));
					}
				}
			}, new_mail_p(this));
		}

		void update()
		{
			if (!drop_tips.empty())
			{
				for (auto p : drop_tips)
				{
					std::vector<Vec2f> points;
					path_rect(points, p.pos, p.size);
					element->canvas->fill(points, p.col);
				}

				drop_tips.clear();
			}
		}

		Component* copy()
		{
			auto copy = new cDockerTabPrivate();

			copy->root = root;

			return copy;
		}
	};

	void cDockerTab::take_away(bool close)
	{
		((cDockerTabPrivate*)this)->take_away(close);
	}

	void cDockerTab::start()
	{
		((cDockerTabPrivate*)this)->start();
	}

	void cDockerTab::update()
	{
		((cDockerTabPrivate*)this)->update();
	}

	Component* cDockerTab::copy()
	{
		return ((cDockerTabPrivate*)this)->copy();
	}

	cDockerTab* cDockerTab::create()
	{
		return new cDockerTabPrivate;
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
			if (!entity->dying)
			{
				event_receiver->drag_and_drop_listeners.remove(drag_and_drop_listener);
				list->remove_selected_changed_listener(selected_changed_listener);
			}
		}

		uint calc_pos(float x, float* out)
		{
			for (auto i = 0; i < entity->child_count(); i++)
			{
				auto element = (cElement*)entity->child(i)->find_component(cH("Element"));
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
				auto element = (cElement*)entity->child(entity->child_count() - 1)->find_component(cH("Element"));
				*out = element->global_pos.x() + element->global_size.x();
			}
			return entity->child_count();
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			event_receiver->set_acceptable_drops({ cH("DockerTab") });
			list = (cList*)(entity->find_component(cH("List")));
			assert(list);
			assert(entity->parent()->name_hash() == cH("docker"));

			drag_and_drop_listener = event_receiver->drag_and_drop_listeners.add([](void* c, DragAndDrop action, cEventReceiver* er, const Vec2i& pos) {
				auto thiz = (*(cDockerTabbarPrivate**)c);
				if (thiz->entity->child_count() > 0) // a valid docker tabbar must have at least one item
				{
					if (action == DragOvering)
					{
						float show_drop_pos;
						auto idx = thiz->calc_pos(pos.x(), &show_drop_pos);
						if (idx == thiz->entity->child_count())
							show_drop_pos -= 10.f;
						else if (idx != 0)
							show_drop_pos -= 5.f;
						auto drop_tab = (cDockerTabPrivate*)(er->entity->find_component(cH("DockerTab")));
						cDockerTabPrivate::Primitive primitive;
						primitive.col = Vec4c(50, 80, 200, 200);
						primitive.pos = Vec2f(show_drop_pos, thiz->element->global_pos.y());
						primitive.size = Vec2f(10.f, thiz->element->global_size.y());
						drop_tab->drop_tips.push_back(primitive);
					}
					else if (action == Dropped)
					{
						thiz->drop_tab = (cDockerTab*)(er->entity->find_component(cH("DockerTab")));
						thiz->drop_idx = thiz->calc_pos(pos.x(), nullptr);
						thiz->drop_tab->floating = false;
						looper().add_delay_event([](void* c) {
							auto thiz = *(cDockerTabbarPrivate**)c;
							auto tabbar = thiz->entity;
							auto tab = thiz->drop_tab;
							auto e_tab = tab->entity;
							auto e_page = tab->page;
							auto page_element = tab->page_element;
							auto page_aligner = (cAligner*)e_page->find_component(cH("Aligner"));

							tab->root->remove_child(e_tab, false);
							tab->root->remove_child(e_page, false);
							tab->list_item->list = thiz->list;
							tabbar->add_child(e_tab, thiz->drop_idx);
							tabbar->parent()->child(1)->add_child(e_page);

							tab->element->alpha = 1.f;
							e_page->visible = false;
							page_element->pos = 0.f;
							page_element->alpha = 1.f;
							page_aligner->width_policy = SizeFitParent;
							page_aligner->height_policy = SizeFitParent;

							thiz->drop_tab->page = nullptr;
							thiz->drop_tab->page_element = nullptr;
							thiz->drop_tab = nullptr;
							thiz->drop_idx = 0;
						}, new_mail_p(thiz));
					}
				}
			}, new_mail_p(this));

			selected_changed_listener = list->add_selected_changed_listener([](void* c, Entity* selected) {
				auto thiz = (*(cDockerTabbarPrivate**)c);
				auto tabbar = thiz->entity;
				auto docker = tabbar->parent();
				auto pages = docker->child(1);
				auto idx = tabbar->child_position(selected);
				for (auto i = 0; i < pages->child_count(); i++)
					pages->child(i)->visible = false;
				pages->child(idx)->visible = true;
			}, new_mail_p(this));

			list->set_selected(entity->child(0));
		}
	};

	void cDockerTabbar::start()
	{
		((cDockerTabbarPrivate*)this)->start();
	}

	void cDockerTabbar::update()
	{
	}

	Component* cDockerTabbar::copy()
	{
		return new cDockerTabbarPrivate;
	}

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
			if (!entity->dying)
				event_receiver->drag_and_drop_listeners.remove(drag_and_drop_listener);
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			event_receiver->set_acceptable_drops({ cH("DockerTab") });

			drag_and_drop_listener = event_receiver->drag_and_drop_listeners.add([](void* c, DragAndDrop action, cEventReceiver* er, const Vec2i& pos) {
				auto thiz = (*(cDockerPagesPrivate**)c);
				if (action == DragOvering)
				{
					auto element = thiz->element;
					thiz->dock_side = Outside;
					auto center = element->global_pos + element->global_size * 0.5f;
					if (rect_contains(Vec4f(center + Vec2f(-25.f), center + Vec2f(25.f)), (Vec2f)pos))
						thiz->dock_side = SideCenter;
					else if (rect_contains(Vec4f(center + Vec2f(-55.f, -25.f), center + Vec2f(-30.f, 25.f)), (Vec2f)pos))
						thiz->dock_side = SideW;
					else if (rect_contains(Vec4f(center + Vec2f(30.f, -25.f), center + Vec2f(55.f, 25.f)), (Vec2f)pos))
						thiz->dock_side = SideE;
					else if (rect_contains(Vec4f(center + Vec2f(-25.f, -55.f), center + Vec2f(25.f, -30.f)), (Vec2f)pos))
						thiz->dock_side = SideN;
					else if (rect_contains(Vec4f(center + Vec2f(-25.f, 30.f), center + Vec2f(25.f, 55.f)), (Vec2f)pos))
						thiz->dock_side = SideS;

					auto drop_tab = (cDockerTabPrivate*)(er->entity->find_component(cH("DockerTab")));
					{
						cDockerTabPrivate::Primitive primitive;
						primitive.col = (thiz->dock_side == SideCenter ? Vec4c(60, 90, 210, 255) : Vec4c(50, 80, 200, 200));
						primitive.pos = center + Vec2f(-25.f);
						primitive.size = Vec2f(50.f);
						std::vector<Vec2f> points;
						drop_tab->drop_tips.push_back(primitive);
					}
					{
						cDockerTabPrivate::Primitive primitive;
						primitive.col = (thiz->dock_side == SideW ? Vec4c(60, 90, 210, 255) : Vec4c(50, 80, 200, 200));
						primitive.pos = center + Vec2f(-55.f, -25.f);
						primitive.size = Vec2f(25.f, 50.f);
						std::vector<Vec2f> points;
						drop_tab->drop_tips.push_back(primitive);
					}
					{
						cDockerTabPrivate::Primitive primitive;
						primitive.col = (thiz->dock_side == SideE ? Vec4c(60, 90, 210, 255) : Vec4c(50, 80, 200, 200));
						primitive.pos = center + Vec2f(30.f, -25.f);
						primitive.size = Vec2f(25.f, 50.f);
						std::vector<Vec2f> points;
						drop_tab->drop_tips.push_back(primitive);
					}
					{
						cDockerTabPrivate::Primitive primitive;
						primitive.col = (thiz->dock_side == SideN ? Vec4c(60, 90, 210, 255) : Vec4c(50, 80, 200, 200));
						primitive.pos = center + Vec2f(-25.f, -55.f);
						primitive.size = Vec2f(50.f, 25.f);
						std::vector<Vec2f> points;
						drop_tab->drop_tips.push_back(primitive);
					}
					{
						cDockerTabPrivate::Primitive primitive;
						primitive.col = (thiz->dock_side == SideS ? Vec4c(60, 90, 210, 255) : Vec4c(50, 80, 200, 200));
						primitive.pos = center + Vec2f(-25.f, 30.f);
						primitive.size = Vec2f(50.f, 25.f);
						std::vector<Vec2f> points;
						drop_tab->drop_tips.push_back(primitive);
					}
				}
				else if (action == Dropped)
				{
					if (thiz->dock_side != Outside)
					{
						if (thiz->dock_side == SideCenter)
						{
							auto tabbar_er = (cEventReceiverPrivate*)thiz->entity->parent()->child(0)->find_component(cH("EventReceiver"));
							tabbar_er->on_drag_and_drop(Dropped, er, Vec2i(0, 99999));
						}
						else
						{
							thiz->drop_tab = (cDockerTab*)(er->entity->find_component(cH("DockerTab")));
							thiz->drop_tab->floating = false;
							looper().add_delay_event([](void* c) {
								auto thiz = (*(cDockerPagesPrivate**)c);
								auto tab = thiz->drop_tab;
								auto e_tab = tab->entity;
								auto e_page = tab->page;
								auto page_element = tab->page_element;
								auto page_aligner = (cAligner*)e_page->find_component(cH("Aligner"));
								auto docker = thiz->entity->parent();
								auto docker_element = (cElement*)docker->find_component(cH("Element"));
								auto docker_aligner = (cAligner*)docker->find_component(cH("Aligner"));
								auto p = docker->parent();
								auto docker_idx = p->child_position(docker);
								auto layout = get_docker_layout_model()->copy();

								if (p->name_hash() == cH("docker_container"))
								{
									auto aligner = (cAligner*)docker->find_component(cH("Aligner"));
									aligner->x_align = AlignxFree;
									aligner->y_align = AlignyFree;
									aligner->using_padding = false;
								}
								else
								{
									auto p_element = (cElement*)p->find_component(cH("Element"));
									auto layout_element = (cElement*)layout->find_component(cH("Element"));

									layout_element->size = p_element->size;

									auto aligner = (cAligner*)layout->find_component(cH("Aligner"));
									aligner->x_align = AlignxFree;
									aligner->y_align = AlignyFree;
									aligner->width_factor = p_element->size.x();
									aligner->height_factor = p_element->size.y();
									aligner->using_padding = false;

									{
										auto oth = p->child(docker_idx == 0 ? 2 : 0);
										auto element = (cElement*)oth->find_component(cH("Element"));
										auto aligner = (cAligner*)oth->find_component(cH("Aligner"));
										aligner->width_factor = element->size.x();
										aligner->height_factor = element->size.y();
									}
								}
								p->remove_child(docker, false);
								p->add_child(layout, docker_idx);

								auto new_docker = get_docker_model()->copy();
								auto new_docker_element = (cElement*)new_docker->find_component(cH("Element"));
								auto new_docker_aligner = (cAligner*)new_docker->find_component(cH("Aligner"));
								{
									auto c_aligner = (cAligner*)new_docker->find_component(cH("Aligner"));
									c_aligner->x_align = AlignxFree;
									c_aligner->y_align = AlignyFree;
									c_aligner->using_padding = false;
								}
								auto new_tabbar = new_docker->child(0);
								auto new_pages = new_docker->child(1);

								tab->root->remove_child(e_tab, false);
								tab->root->remove_child(e_page, false);
								tab->list_item->list = (cList*)new_tabbar->find_component(cH("List"));
								new_tabbar->add_child(e_tab);
								new_pages->add_child(e_page);

								tab->element->alpha = 1.f;
								e_page->visible = false;
								page_element->pos = 0.f;
								page_element->alpha = 1.f;
								page_aligner->width_policy = SizeFitParent;
								page_aligner->height_policy = SizeFitParent;

								auto e_splitter = layout->child(0);
								auto splitter_element = (cElement*)e_splitter->find_component(cH("Element"));
								auto splitter = (cSplitter*)e_splitter->find_component(cH("Splitter"));
								if (thiz->dock_side == SideW || thiz->dock_side == SideE)
								{
									auto w = (docker_element->size.x() - splitter_element->size.x()) * 0.5f;
									docker_element->size.x() = w;
									new_docker_element->size.x() = w;
									docker_aligner->width_factor = w;
									new_docker_aligner->width_factor = w;

									auto h = docker_element->size.y() * 0.5f;
									docker_element->size.y() = h;
									new_docker_element->size.y() = h;
									docker_aligner->height_factor = h;
									new_docker_aligner->height_factor = h;
								}
								else
								{
									((cLayout*)layout->find_component(cH("Layout")))->type = LayoutVertical;
									splitter_element->size.y() = splitter_element->size.x();
									splitter_element->size.x() = 0.f;
									splitter->type = SplitterVertical;
									auto splitter_aligner = (cAligner*)e_splitter->find_component(cH("Aligner"));
									splitter_aligner->width_policy = SizeFitParent;
									splitter_aligner->height_policy = SizeFixed;

									auto w = docker_element->size.x();
									docker_element->size.x() = w;
									new_docker_element->size.x() = w;
									docker_aligner->width_factor = w;
									new_docker_aligner->width_factor = w;

									auto h = (docker_element->size.y() - splitter_element->size.y()) * 0.5f;
									docker_element->size.y() = h;
									new_docker_element->size.y() = h;
									docker_aligner->height_factor = h;
									new_docker_aligner->height_factor = h;
								}

								if (thiz->dock_side == SideW)
								{
									layout->add_child(new_docker, 0);
									layout->add_child(docker, 2);
								}
								else if (thiz->dock_side == SideE)
								{
									layout->add_child(docker, 0);
									layout->add_child(new_docker, 2);
								}
								else if (thiz->dock_side == SideN)
								{
									layout->add_child(new_docker, 0);
									layout->add_child(docker, 2);
								}
								else if (thiz->dock_side == SideS)
								{
									layout->add_child(docker, 0);
									layout->add_child(new_docker, 2);
								}
							}, new_mail_p(thiz));
						}
					}
				}
			}, new_mail_p(this));
		}
	};

	void cDockerPages::start()
	{
		((cDockerPagesPrivate*)this)->start();
	}

	void cDockerPages::update()
	{
	}

	Component* cDockerPages::copy()
	{
		return new cDockerPagesPrivate;
	}

	cDockerPages* cDockerPages::create()
	{
		return new cDockerPagesPrivate;
	}

	Entity* create_standard_docker_tab(graphics::FontAtlas* font_atlas, const std::wstring& title, Entity* root)
	{
		auto tab = Entity::create();
		tab->set_name("docker_tab");

		auto c_element = cElement::create();
		c_element->inner_padding = Vec4f(4.f, 2.f, 6.f + font_atlas->pixel_height, 2.f);
		tab->add_component(c_element);

		auto c_text = cText::create(font_atlas);
		c_text->set_text(title);
		tab->add_component(c_text);

		tab->add_component(cEventReceiver::create());

		auto c_docker_tab = cDockerTab::create();
		c_docker_tab->root = root;
		tab->add_component(c_docker_tab);

		tab->add_component(cStyleColor::create());
		tab->add_component(cStyleTextColor::create());

		auto list_item = cListItem::create();
		list_item->unselected_color_normal = default_style.tab_color_normal;
		list_item->unselected_color_hovering = default_style.tab_color_else;
		list_item->unselected_color_active = default_style.tab_color_else;
		list_item->unselected_text_color_normal = default_style.tab_text_color_normal;
		list_item->unselected_text_color_else = default_style.tab_text_color_else;
		list_item->selected_color_normal = default_style.selected_tab_color_normal;
		list_item->selected_color_hovering = default_style.selected_tab_color_else;
		list_item->selected_color_active = default_style.selected_tab_color_else;
		list_item->selected_text_color_normal = default_style.selected_tab_text_color_normal;
		list_item->selected_text_color_else = default_style.selected_tab_text_color_else;
		tab->add_component(list_item);

		tab->add_component(cLayout::create(LayoutFree));

		auto e_close = Entity::create();
		tab->add_child(e_close);
		{
			auto c_element = cElement::create();
			c_element->inner_padding = Vec4f(2.f, 2.f, 4.f, 2.f);
			e_close->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->set_text(Icon_WINDOW_CLOSE);
			e_close->add_component(c_text);

			auto c_event_receiver = cEventReceiver::create();
			c_event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				if (is_mouse_clicked(action, key))
				{
					auto thiz = (*(cDockerTabPrivate**)c);
					looper().add_delay_event([](void* c) {
						auto thiz = (*(cDockerTabPrivate**)c);
						thiz->take_away(true);
					}, new_mail_p(thiz));
				}
			}, new_mail_p(c_docker_tab));
			e_close->add_component(c_event_receiver);

			auto c_aligner = cAligner::create();
			c_aligner->x_align = AlignxRight;
			e_close->add_component(c_aligner);
		}

		return tab;
	}

	static Entity* docker_page_model;
	Entity* get_docker_page_model()
	{
		if (!docker_page_model)
		{
			docker_page_model = Entity::create();
			docker_page_model->set_name("docker_page");

			auto c_element = cElement::create();
			c_element->color = default_style.window_color;
			c_element->clip_children = true;
			docker_page_model->add_component(c_element);

			auto c_aligner = cAligner::create();
			c_aligner->width_policy = SizeFitParent;
			c_aligner->height_policy = SizeFitParent;
			docker_page_model->add_component(c_aligner);
		}
		return docker_page_model;
	}

	static Entity* docker_model;
	Entity* get_docker_model()
	{
		if (!docker_model)
		{
			docker_model = Entity::create();
			docker_model->set_name("docker");

			docker_model->add_component(cElement::create());

			auto c_aligner = cAligner::create();
			c_aligner->x_align = AlignxLeft;
			c_aligner->y_align = AlignyTop;
			c_aligner->width_policy = SizeFitParent;
			c_aligner->height_policy = SizeFitParent;
			c_aligner->using_padding = true;
			docker_model->add_component(c_aligner);

			auto c_layout = cLayout::create(LayoutVertical);
			c_layout->width_fit_children = false;
			c_layout->height_fit_children = false;
			docker_model->add_component(c_layout);

			auto e_tabbar = Entity::create();
			e_tabbar->set_name("docker_tabbar");
			docker_model->add_child(e_tabbar);
			{
				auto c_element = cElement::create();
				c_element->clip_children = true;
				e_tabbar->add_component(c_element);

				e_tabbar->add_component(cEventReceiver::create());

				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitParent;
				e_tabbar->add_component(c_aligner);

				e_tabbar->add_component(cLayout::create(LayoutHorizontal));

				e_tabbar->add_component(cList::create(false));

				e_tabbar->add_component(cDockerTabbar::create());
			}

			auto e_pages = Entity::create();
			e_pages->set_name("docker_pages");
			docker_model->add_child(e_pages);
			{
				e_pages->add_component(cElement::create());

				e_pages->add_component(cEventReceiver::create());

				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitParent;
				c_aligner->height_policy = SizeFitParent;
				e_pages->add_component(c_aligner);

				e_pages->add_component(cLayout::create(LayoutFree));

				e_pages->add_component(cDockerPages::create());
			}
		}
		return docker_model;
	}

	static Entity* docker_layout_model;
	Entity* get_docker_layout_model()
	{
		if (!docker_layout_model)
		{
			docker_layout_model = Entity::create();
			docker_layout_model->set_name("docker_layout");

			docker_layout_model->add_component(cElement::create());

			auto c_aligner = cAligner::create();
			c_aligner->x_align = AlignxLeft;
			c_aligner->y_align = AlignyTop;
			c_aligner->width_policy = SizeFitParent;
			c_aligner->height_policy = SizeFitParent;
			c_aligner->using_padding = true;
			docker_layout_model->add_component(c_aligner);

			auto c_layout = cLayout::create(LayoutHorizontal);
			c_layout->width_fit_children = false;
			c_layout->height_fit_children = false;
			docker_layout_model->add_component(c_layout);

			auto e_spliter = Entity::create();
			docker_layout_model->add_child(e_spliter);
			{
				auto c_element = cElement::create();
				c_element->size.x() = 8.f;
				e_spliter->add_component(c_element);

				e_spliter->add_component(cEventReceiver::create());

				e_spliter->add_component(cStyleColor::create(Vec4c(0), default_style.frame_color_hovering, default_style.frame_color_active));

				e_spliter->add_component(cSplitter::create());

				auto c_aligner = cAligner::create();
				c_aligner->height_policy = SizeFitParent;
				e_spliter->add_component(c_aligner);
			}

		}
		return docker_layout_model;
	}

	static Entity* docker_container_model;
	Entity* get_docker_container_model()
	{
		if (!docker_container_model)
		{
			docker_container_model = Entity::create();
			docker_container_model->set_name("docker_container");

			auto c_element = cElement::create();
			c_element->size = 200.f;
			c_element->inner_padding = Vec4f(8.f, 16.f, 8.f, 8.f);
			c_element->color = default_style.docker_color;
			docker_container_model->add_component(c_element);

			docker_container_model->add_component(cEventReceiver::create());

			docker_container_model->add_component(cLayout::create(LayoutFree));

			docker_container_model->add_component(cMoveable::create());

			auto e_bring_to_front = Entity::create();
			docker_container_model->add_child(e_bring_to_front);
			{
				e_bring_to_front->add_component(cElement::create());

				auto c_event_receiver = cEventReceiver::create();
				c_event_receiver->penetrable = true;
				e_bring_to_front->add_component(c_event_receiver);

				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitParent;
				c_aligner->height_policy = SizeFitParent;
				e_bring_to_front->add_component(c_aligner);

				e_bring_to_front->add_component(cBringToFront::create());
			}

			auto e_size_dragger = Entity::create();
			docker_container_model->add_child(e_size_dragger);
			{
				auto c_element = cElement::create();
				c_element->size = 8.f;
				c_element->color = Vec4c(200, 100, 100, 255);
				e_size_dragger->add_component(c_element);

				e_size_dragger->add_component(cEventReceiver::create());

				auto c_aligner = cAligner::create();
				c_aligner->x_align = AlignxRight;
				c_aligner->y_align = AlignyBottom;
				e_size_dragger->add_component(c_aligner);

				e_size_dragger->add_component(cSizeDragger::create());
			}
		}
		return docker_container_model;
	}
}
