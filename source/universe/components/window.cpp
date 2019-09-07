#include <flame/graphics/canvas.h>
#include <flame/universe/topmost.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_dispatcher.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/list.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/splitter.h>
#include <flame/universe/components/window.h>

namespace flame
{
	struct cWindowPrivate : cWindow
	{
		void* mouse_listener;

		std::vector<std::unique_ptr<Closure<void(void* c)>>> pos_listeners;

		cWindowPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			mouse_listener = nullptr;
		}

		~cWindowPrivate()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			if (event_receiver)
				event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto thiz = (*(cWindowPrivate**)c);
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					looper().add_delay_event([](void* c) {
						auto e = *(Entity**)c;
						e->parent()->reposition_child(e, -1);
					}, new_mail_p(thiz->entity));
				}
				else if (thiz->event_receiver->active && is_mouse_move(action, key))
				{
					auto e = thiz->element;
					auto x = pos.x() / e->global_scale;
					auto y = pos.y() / e->global_scale;
					e->x += x;
					e->y += y;
					for (auto& l : thiz->pos_listeners)
						l->function(l->capture.p);
				}
			}, new_mail_p(this));
		}
	};

	void cWindow::start()
	{
		((cWindowPrivate*)this)->start();
	}

	void cWindow::update() 
	{
	}

	Component* cWindow::copy()
	{
		return new cWindowPrivate;
	}

	void* cWindow::add_pos_listener(void (*listener)(void* c), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c)>;
		c->function = listener;
		c->capture = capture;
		((cWindowPrivate*)this)->pos_listeners.emplace_back(c);
		return c;
	}

	void cWindow::remove_pos_listener(void* ret_by_add)
	{
		auto& listeners = ((cWindowPrivate*)this)->pos_listeners;
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == ret_by_add)
			{
				listeners.erase(it);
				return;
			}
		}
	}

	cWindow* cWindow::create()
	{
		return new cWindowPrivate;
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
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			if (event_receiver)
				event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			p_element = (cElement*)(entity->parent()->find_component(cH("Element")));
			assert(p_element);

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto thiz = (*(cSizeDraggerPrivate**)c);
				if (is_mouse_move(action, key) && thiz->event_receiver->active)
				{
					thiz->p_element->width += pos.x();
					thiz->p_element->height += pos.y();
				}
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

		cDockerTabPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;
			list_item = nullptr;

			root = nullptr;

			page = nullptr;

			mouse_listener = nullptr;
			drag_and_drop_listener = nullptr;
			drop_pos = Vec2f(0.f);
		}

		~cDockerTabPrivate()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			if (event_receiver)
			{
				event_receiver->remove_mouse_listener(mouse_listener);
				event_receiver->remove_drag_and_drop_listener(drag_and_drop_listener);
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

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto thiz = (*(cDockerTabPrivate**)c);
				if (is_mouse_move(action, key) && thiz->event_receiver->dragging && thiz->page)
				{
					thiz->element->x += pos.x();
					thiz->element->y += pos.y();
					thiz->page_element->x += pos.x();
					thiz->page_element->y += pos.y();
				}
			}, new_mail_p(this));

			drag_and_drop_listener = event_receiver->add_drag_and_drop_listener([](void* c, DragAndDrop action, cEventReceiver* er, const Vec2f& pos) {
				auto thiz = (*(cDockerTabPrivate**)c);
				if (action == DragStart)
				{
					auto list = thiz->list_item->list;
					if (list && list->selected == thiz->entity)
						list->set_selected(list->entity->child(0));
					thiz->list_item->list = nullptr;
					looper().add_delay_event([](void* c) {
						auto thiz = *(cDockerTabPrivate**)c;

						auto e = thiz->entity;
						auto tabbar = e->parent();
						if (tabbar->name_hash() != cH("docker_tabbar"))
							return;

						auto docker = tabbar->parent();
						auto pages = docker->child(1);
						auto idx = tabbar->child_position(e);
						auto e_page = pages->child(idx);
						auto page_element = (cElement*)e_page->find_component(cH("Element"));
						auto page_aligner = (cAligner*)e_page->find_component(cH("Aligner"));
						thiz->page = e_page;
						thiz->page_element = page_element;

						tabbar->take_child(e);
						pages->take_child(e_page);
						e_page->visible = true;

						if (tabbar->child_count() == 0)
						{
							auto p = docker->parent();
							if (p)
							{
								if (p->name_hash() == cH("docker_container"))
									p->parent()->remove_child(p);
								else if (p->name_hash() == cH("docker_layout"))
								{
									auto idx = p->child_position(docker);
									auto oth_docker = p->child(idx == 0 ? 2 : 0);
									p->take_child(oth_docker);
									auto pp = p->parent();
									pp->remove_child(p);
									pp->add_child(oth_docker);
									if (pp->name_hash() == cH("docker_container"))
									{
										auto aligner = (cAligner*)oth_docker->find_component(cH("Aligner"));
										aligner->x_align = AlignxLeft;
										aligner->y_align = AlignyTop;
										aligner->using_padding_in_free_layout = true;
									}
								}
							}
						}

						auto element = thiz->element;
						element->x = element->global_x;
						element->y = element->global_y;
						element->alpha = 0.5f;
						thiz->root->add_child(e);
						page_element->x = element->x;
						page_element->y = element->y + element->global_height + 8.f;
						page_element->alpha = 0.5f;
						page_aligner->width_policy = SizeFixed;
						page_aligner->height_policy = SizeFixed;
						thiz->root->add_child(e_page);
					}, new_mail_p(thiz));
				}
				else if (action == DragEnd)
				{
					if (!er)
					{
						thiz->drop_pos = pos;
						looper().add_delay_event([](void* c) {
							auto thiz = (*(cDockerTabPrivate**)c);

							auto e_container = get_docker_container_model()->copy();
							thiz->root->add_child(e_container);
							{
								auto c_element = (cElement*)e_container->find_component(cH("Element"));
								c_element->x = thiz->drop_pos.x();
								c_element->y = thiz->drop_pos.y();
							}

							auto e_docker = get_docker_model()->copy();
							e_container->add_child(e_docker);
							auto e_tabbar = e_docker->child(0);
							auto e_pages = e_docker->child(1);

							auto e_tab = thiz->entity;
							auto e_page = thiz->page;
							auto page_element = thiz->page_element;
							auto page_aligner = (cAligner*)e_page->find_component(cH("Aligner"));

							thiz->root->take_child(e_tab);
							thiz->root->take_child(e_page);
							thiz->list_item->list = (cList*)e_tabbar->find_component(cH("List"));
							e_tabbar->add_child(e_tab);
							auto element = thiz->element;
							element->x = 0;
							element->y = 0;
							element->alpha = 1.f;
							e_pages->add_child(e_page);
							page_element->x = 0;
							page_element->y = 0;
							page_element->alpha = 1.f;
							page_aligner->width_policy = SizeFitLayout;
							page_aligner->height_policy = SizeFitLayout;
							thiz->page = nullptr;
							thiz->page_element = nullptr;
						}, new_mail_p(thiz));
					}
				}
			}, new_mail_p(this));
		}

		Component* copy()
		{
			auto copy = new cDockerTabPrivate();

			copy->root = root;

			return copy;
		}
	};

	void cDockerTab::start()
	{
		((cDockerTabPrivate*)this)->start();
	}

	void cDockerTab::update()
	{
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
		bool show_drop_tip;
		float show_drop_pos;
		void* selected_changed_listener;

		cDockerTabbarPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;
			list = nullptr;

			drag_and_drop_listener = nullptr;
			drop_tab = nullptr;
			drop_idx = 0;
			show_drop_tip = false;
			show_drop_pos = 0.f;
		}

		~cDockerTabbarPrivate()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			if (event_receiver)
				event_receiver->remove_drag_and_drop_listener(drag_and_drop_listener);
			list = (cList*)(entity->find_component(cH("List")));
			if (list)
				list->remove_selected_changed_listener(selected_changed_listener);
		}

		uint calc_pos(float x, float* out)
		{
			for (auto i = 0; i < entity->child_count(); i++)
			{
				auto element = (cElement*)entity->child(i)->find_component(cH("Element"));
				auto half = element->global_x + element->width * 0.5f;
				if (x >= element->global_x && x < half)
				{
					if (out)
						*out = element->global_x;
					return i;
				}
				if (x >= half && x < element->global_x + element->width)
				{
					if (out)
						*out = element->global_x + element->width;
					return i + 1;
				}
			}
			if (out)
			{
				auto element = (cElement*)entity->child(entity->child_count() - 1)->find_component(cH("Element"));
				*out = element->global_x + element->global_width;
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

			drag_and_drop_listener = event_receiver->add_drag_and_drop_listener([](void* c, DragAndDrop action, cEventReceiver* er, const Vec2f& pos) {
				auto thiz = (*(cDockerTabbarPrivate**)c);
				if (thiz->entity->child_count() > 0) // a valid docker tabbar must have at least one item
				{
					if (action == DragOvering)
					{
						auto idx = thiz->calc_pos(pos.x(), &thiz->show_drop_pos);
						if (idx == thiz->entity->child_count())
							thiz->show_drop_pos -= 10.f;
						else if (idx != 0)
							thiz->show_drop_pos -= 5.f;
						thiz->show_drop_tip = true;
					}
					else if (action == Dropped)
					{
						thiz->drop_tab = (cDockerTab*)(er->entity->find_component(cH("DockerTab")));
						thiz->drop_idx = thiz->calc_pos(pos.x(), nullptr);
						looper().add_delay_event([](void* c) {
							auto thiz = *(cDockerTabbarPrivate**)c;
							auto tabbar = thiz->entity;
							auto tab = thiz->drop_tab;
							auto e_tab = tab->entity;
							auto e_page = tab->page;
							auto page_element = tab->page_element;
							auto page_aligner = (cAligner*)e_page->find_component(cH("Aligner"));

							tab->root->take_child(e_tab);
							tab->root->take_child(e_page);
							tab->list_item->list = thiz->list;
							tabbar->add_child(e_tab, thiz->drop_idx);
							tabbar->parent()->child(1)->add_child(e_page);

							tab->element->alpha = 1.f;
							e_page->visible = false;
							page_element->x = 0;
							page_element->y = 0;
							page_element->alpha = 1.f;
							page_aligner->width_policy = SizeFitLayout;
							page_aligner->height_policy = SizeFitLayout;

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

		void update()
		{
			if (show_drop_tip)
			{
				show_drop_tip = false;
				std::vector<Vec2f> points;
				path_rect(points, Vec2f(show_drop_pos, element->global_y), Vec2f(10.f, element->global_height));
				element->canvas->fill(points, Vec4c(50, 80, 200, 200));
			}
		}
	};

	void cDockerTabbar::start()
	{
		((cDockerTabbarPrivate*)this)->start();
	}

	void cDockerTabbar::update()
	{
		((cDockerTabbarPrivate*)this)->update();
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
		bool show_drop_tip;

		cDockerPagesPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			drag_and_drop_listener = nullptr;
			drop_tab = nullptr;
			dock_side = Outside;
			show_drop_tip = false;
		}

		~cDockerPagesPrivate()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			if (event_receiver)
				event_receiver->remove_drag_and_drop_listener(drag_and_drop_listener);
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			event_receiver->set_acceptable_drops({ cH("DockerTab") });

			drag_and_drop_listener = event_receiver->add_drag_and_drop_listener([](void* c, DragAndDrop action, cEventReceiver* er, const Vec2f& pos) {
				auto thiz = (*(cDockerPagesPrivate**)c);
				if (action == DragOvering)
				{
					auto element = thiz->element;
					thiz->dock_side = Outside;
					auto center = Vec2f(element->global_x + element->global_width * 0.5f, element->global_y + element->global_height * 0.5f);
					if (rect_contains(Vec4f(center + Vec2f(-25.f), center + Vec2f(25.f)), pos))
						thiz->dock_side = SideCenter;
					else if (rect_contains(Vec4f(center + Vec2f(-55.f, -25.f), center + Vec2f(-30.f, 25.f)), pos))
						thiz->dock_side = SideW;
					else if (rect_contains(Vec4f(center + Vec2f(30.f, -25.f), center + Vec2f(55.f, 25.f)), pos))
						thiz->dock_side = SideE;
					else if (rect_contains(Vec4f(center + Vec2f(-25.f, -55.f), center + Vec2f(25.f, -30.f)), pos))
						thiz->dock_side = SideN;
					else if (rect_contains(Vec4f(center + Vec2f(-25.f, 30.f), center + Vec2f(25.f, 55.f)), pos))
						thiz->dock_side = SideS;
					thiz->show_drop_tip = true;
				}
				else if (action == Dropped)
				{
					if (thiz->dock_side != Outside)
					{
						if (thiz->dock_side == SideCenter)
						{
							auto tabbar_er = (cEventReceiver*)thiz->entity->parent()->child(0)->find_component(cH("EventReceiver"));
							tabbar_er->on_drag_and_drop(Dropped, er, Vec2f(0.f, 99999.f));
						}
						else
						{
							thiz->drop_tab = (cDockerTab*)(er->entity->find_component(cH("DockerTab")));
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
								auto layout = get_docker_layout_model()->copy();

								if (p->name_hash() == cH("docker_container"))
								{
									auto aligner = (cAligner*)docker->find_component(cH("Aligner"));
									aligner->x_align = AlignxFree;
									aligner->y_align = AlignyFree;
									aligner->using_padding_in_free_layout = false;
								}
								else
								{
									auto p_element = (cElement*)p->find_component(cH("Element"));
									auto layout_element = (cElement*)layout->find_component(cH("Element"));

									layout_element->width = p_element->width;
									layout_element->height = p_element->height;

									auto aligner = (cAligner*)layout->find_component(cH("Aligner"));
									aligner->x_align = AlignxFree;
									aligner->y_align = AlignyFree;
									aligner->width_factor = p_element->width;
									aligner->height_factor = p_element->height;
									aligner->using_padding_in_free_layout = false;
								}
								{
									auto pos = p->child_position(docker);
									p->take_child(docker);
									p->add_child(layout, pos);
								}

								auto new_docker = get_docker_model()->copy();
								auto new_docker_element = (cElement*)new_docker->find_component(cH("Element"));
								auto new_docker_aligner = (cAligner*)new_docker->find_component(cH("Aligner"));
								{
									auto c_aligner = (cAligner*)new_docker->find_component(cH("Aligner"));
									c_aligner->x_align = AlignxFree;
									c_aligner->y_align = AlignyFree;
									c_aligner->using_padding_in_free_layout = false;
								}
								auto new_tabbar = new_docker->child(0);
								auto new_pages = new_docker->child(1);

								tab->root->take_child(e_tab);
								tab->root->take_child(e_page);
								tab->list_item->list = (cList*)new_tabbar->find_component(cH("List"));
								new_tabbar->add_child(e_tab);
								new_pages->add_child(e_page);

								tab->element->alpha = 1.f;
								e_page->visible = false;
								page_element->x = 0;
								page_element->y = 0;
								page_element->alpha = 1.f;
								page_aligner->width_policy = SizeFitLayout;
								page_aligner->height_policy = SizeFitLayout;

								auto e_splitter = layout->child(0);
								auto splitter_element = (cElement*)e_splitter->find_component(cH("Element"));
								auto splitter = (cSplitter*)e_splitter->find_component(cH("Splitter"));
								if (thiz->dock_side == SideW || thiz->dock_side == SideE)
								{
									auto w = (docker_element->width - splitter_element->width) * 0.5f;
									docker_element->width = w;
									new_docker_element->width = w;
									docker_aligner->width_factor = w;
									new_docker_aligner->width_factor = w;

									auto h = docker_element->height * 0.5f;
									docker_element->height = h;
									new_docker_element->height = h;
									docker_aligner->height_factor = h;
									new_docker_aligner->height_factor = h;
								}
								else
								{
									((cLayout*)layout->find_component(cH("Layout")))->type = LayoutVertical;
									splitter_element->height = splitter_element->width;
									splitter_element->width = 0.f;
									splitter->type = SplitterVertical;
									auto splitter_aligner = (cAligner*)e_splitter->find_component(cH("Aligner"));
									splitter_aligner->width_policy = SizeFitLayout;
									splitter_aligner->height_policy = SizeFixed;

									auto w = docker_element->width;
									docker_element->width = w;
									new_docker_element->width = w;
									docker_aligner->width_factor = w;
									new_docker_aligner->width_factor = w;

									auto h = (docker_element->height - splitter_element->height) * 0.5f;
									docker_element->height = h;
									new_docker_element->height = h;
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

		void update()
		{
			if (show_drop_tip)
			{
				show_drop_tip = false;
				auto center = Vec2f(element->global_x + element->global_width * 0.5f, element->global_y + element->global_height * 0.5f);
				{
					std::vector<Vec2f> points;
					path_rect(points, center + Vec2f(-25.f), Vec2f(50.f));
					element->canvas->fill(points, dock_side == SideCenter ? Vec4c(60, 90, 210, 255) : Vec4c(50, 80, 200, 200));
				}
				{
					std::vector<Vec2f> points;
					path_rect(points, center + Vec2f(-55.f, -25.f), Vec2f(25, 50.f));
					element->canvas->fill(points, dock_side == SideW ? Vec4c(60, 90, 210, 255) : Vec4c(50, 80, 200, 200));
				}
				{
					std::vector<Vec2f> points;
					path_rect(points, center + Vec2f(30.f, -25.f), Vec2f(25, 50.f));
					element->canvas->fill(points, dock_side == SideE ? Vec4c(60, 90, 210, 255) : Vec4c(50, 80, 200, 200));
				}
				{
					std::vector<Vec2f> points;
					path_rect(points, center + Vec2f(-25.f, -55.f), Vec2f(50, 25.f));
					element->canvas->fill(points, dock_side == SideN ? Vec4c(60, 90, 210, 255) : Vec4c(50, 80, 200, 200));
				}
				{
					std::vector<Vec2f> points;
					path_rect(points, center + Vec2f(-25.f, 30.f), Vec2f(50, 25.f));
					element->canvas->fill(points, dock_side == SideS ? Vec4c(60, 90, 210, 255) : Vec4c(50, 80, 200, 200));
				}
			}
		}
	};

	void cDockerPages::start()
	{
		((cDockerPagesPrivate*)this)->start();
	}

	void cDockerPages::update()
	{
		((cDockerPagesPrivate*)this)->update();
	}

	Component* cDockerPages::copy()
	{
		return new cDockerPagesPrivate;
	}

	cDockerPages* cDockerPages::create()
	{
		return new cDockerPagesPrivate;
	}

	static Entity* docker_tab_model;
	Entity* get_docker_tab_model()
	{
		if (!docker_tab_model)
		{
			docker_tab_model = Entity::create();
			docker_tab_model->set_name("docker_tab");

			auto c_element = cElement::create();
			c_element->inner_padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			docker_tab_model->add_component(c_element);

			docker_tab_model->add_component(cText::create(nullptr));

			docker_tab_model->add_component(cEventReceiver::create());

			docker_tab_model->add_component(cDockerTab::create());

			docker_tab_model->add_component(cStyleBackgroundColor::create(default_style.frame_color_normal, default_style.frame_color_hovering, default_style.frame_color_active));

			docker_tab_model->add_component(cListItem::create());
		}
		return docker_tab_model;
	}

	static Entity* docker_page_model;
	Entity* get_docker_page_model()
	{
		if (!docker_page_model)
		{
			docker_page_model = Entity::create();
			docker_page_model->set_name("docker_page");

			auto c_element = cElement::create();
			c_element->background_frame_color = Vec4c(255);
			c_element->background_frame_thickness = 2.f;
			docker_page_model->add_component(c_element);

			auto c_aligner = cAligner::create();
			c_aligner->width_policy = SizeFitLayout;
			c_aligner->height_policy = SizeFitLayout;
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
			c_aligner->width_policy = SizeFitLayout;
			c_aligner->height_policy = SizeFitLayout;
			c_aligner->using_padding_in_free_layout = true;
			docker_model->add_component(c_aligner);

			auto c_layout = cLayout::create();
			c_layout->type = LayoutVertical;
			c_layout->width_fit_children = false;
			c_layout->height_fit_children = false;
			docker_model->add_component(c_layout);

			auto e_tabbar = Entity::create();
			e_tabbar->set_name("docker_tabbar");
			docker_model->add_child(e_tabbar);
			{
				auto c_element = cElement::create();
				c_element->background_color = Vec4c(0, 0, 0, 255);
				c_element->clip_children = true;
				e_tabbar->add_component(c_element);

				e_tabbar->add_component(cEventReceiver::create());

				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitLayout;
				e_tabbar->add_component(c_aligner);

				auto c_layout = cLayout::create();
				c_layout->type = LayoutHorizontal;
				e_tabbar->add_component(c_layout);

				e_tabbar->add_component(cList::create());

				e_tabbar->add_component(cDockerTabbar::create());
			}

			auto e_pages = Entity::create();
			e_pages->set_name("docker_pages");
			docker_model->add_child(e_pages);
			{
				e_pages->add_component(cElement::create());

				e_pages->add_component(cEventReceiver::create());

				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitLayout;
				c_aligner->height_policy = SizeFitLayout;
				e_pages->add_component(c_aligner);

				auto c_layout = cLayout::create();
				c_layout->width_fit_children = false;
				c_layout->height_fit_children = false;
				e_pages->add_component(c_layout);

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
			c_aligner->width_policy = SizeFitLayout;
			c_aligner->height_policy = SizeFitLayout;
			c_aligner->using_padding_in_free_layout = true;
			docker_layout_model->add_component(c_aligner);

			auto c_layout = cLayout::create();
			c_layout->type = LayoutHorizontal;
			c_layout->width_fit_children = false;
			c_layout->height_fit_children = false;
			docker_layout_model->add_component(c_layout);

			auto e_spliter = Entity::create();
			docker_layout_model->add_child(e_spliter);
			{
				auto c_element = cElement::create();
				c_element->width = 8.f;
				e_spliter->add_component(c_element);

				e_spliter->add_component(cEventReceiver::create());

				e_spliter->add_component(cStyleBackgroundColor::create(Vec4c(0), default_style.frame_color_hovering, default_style.frame_color_active));

				e_spliter->add_component(cSplitter::create());

				auto c_aligner = cAligner::create();
				c_aligner->height_policy = SizeFitLayout;
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
			c_element->width = 200.f;
			c_element->height = 200.f;
			c_element->inner_padding = Vec4f(8.f);
			c_element->background_color = Vec4c(100, 100, 100, 255);
			c_element->background_frame_color = Vec4c(255);
			c_element->background_frame_thickness = 2.f;
			docker_container_model->add_component(c_element);

			docker_container_model->add_component(cEventReceiver::create());

			auto c_layout = cLayout::create();
			c_layout->width_fit_children = false;
			c_layout->height_fit_children = false;
			docker_container_model->add_component(c_layout);

			docker_container_model->add_component(cWindow::create());

			auto e_size_dragger = Entity::create();
			docker_container_model->add_child(e_size_dragger);
			{
				auto c_element = cElement::create();
				c_element->width = 8.f;
				c_element->height = 8.f;
				c_element->background_color = Vec4c(200, 100, 100, 255);
				e_size_dragger->add_component(c_element);

				auto c_aligner = cAligner::create();
				c_aligner->x_align = AlignxRight;
				c_aligner->y_align = AlignyBottom;
				e_size_dragger->add_component(c_aligner);

				e_size_dragger->add_component(cEventReceiver::create());

				e_size_dragger->add_component(cSizeDragger::create());
			}
		}
		return docker_container_model;
	}
}
