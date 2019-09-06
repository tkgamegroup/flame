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

		cDockerTabPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;
			list_item = nullptr;

			root = nullptr;

			page = nullptr;

			mouse_listener = nullptr;
			drag_and_drop_listener = nullptr;
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
				if (action == DragStart)
				{
					auto thiz = (*(cDockerTabPrivate**)c);
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
						thiz->root->add_child(e);
						page_element->x = element->x;
						page_element->y = element->y + element->global_height + 8.f;
						page_aligner->width_policy = SizeFixed;
						page_aligner->height_policy = SizeFixed;
						thiz->root->add_child(e_page);
					}, new_mail_p(thiz));
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
							auto docker = tabbar->parent();
							auto pages = docker->child(1);
							auto tab = thiz->drop_tab;
							auto e_tab = tab->entity;
							auto e_page = tab->page;
							auto page_element = tab->page_element;
							auto page_aligner = (cAligner*)e_page->find_component(cH("Aligner"));
							auto root = tab->root;

							root->take_child(e_tab);
							root->take_child(e_page);
							tab->list_item->list = thiz->list;
							tabbar->add_child(e_tab, thiz->drop_idx);
							pages->add_child(e_page);
							e_page->visible = false;
							page_element->x = 0;
							page_element->y = 0;
							page_aligner->width_policy = SizeFitLayout;
							page_aligner->height_policy = SizeFitLayout;
							thiz->drop_tab->page = nullptr;
							thiz->drop_tab->page_element = nullptr;
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
				element->canvas->fill(points, Vec4c(50, 80, 200, 128));
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

				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitLayout;
				c_aligner->height_policy = SizeFitLayout;
				e_pages->add_component(c_aligner);

				auto c_layout = cLayout::create();
				c_layout->width_fit_children = false;
				c_layout->height_fit_children = false;
				e_pages->add_component(c_layout);
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
