#include "element_private.h"
#include "receiver_private.h"
#include "scroll_view_private.h"
#include "../world_private.h"

namespace flame
{
	cScrollViewPrivate::cScrollViewPrivate()
	{
		content_container.reset();
		content_viewport.reset();
		vertical_scroller.reset();
		vertical_scroller_slider.reset();
	}

	cScrollViewPrivate::~cScrollViewPrivate()
	{
		if (e_content_container)
		{
			e_content_container->message_listeners.remove("scroll_view"_h);
			if (auto element = e_content_container->get_component<cElement>(); element)
				element->data_listeners.remove("scroll_view"_h);
		}
		if (e_content_viewport)
		{
			e_content_viewport->message_listeners.remove("scroll_view"_h);
			if (auto element = e_content_viewport->get_component<cElement>(); element)
				element->data_listeners.remove("scroll_view"_h);
		}
		if (e_vertical_scroller)
		{
			e_vertical_scroller->message_listeners.remove("scroll_view"_h);
			if (auto element = e_vertical_scroller->get_component<cElement>(); element)
				element->data_listeners.remove("scroll_view"_h);
		}
		if (e_vertical_scroller_slider)
		{
			e_vertical_scroller_slider->message_listeners.remove("scroll_view"_h);
			if (auto element = e_vertical_scroller_slider->get_component<cElement>(); element)
				element->data_listeners.remove("scroll_view"_h);
			if (auto receiver = e_vertical_scroller_slider->get_component<cReceiver>(); receiver)
				receiver->event_listeners.remove("scroll_view"_h);
		}
	}

	void cScrollViewPrivate::on_active()
	{
		update_content_container();
		update_content_viewport();
		update_vertical_scroller();
		update_vertical_scroller_slider();
	}

	void cScrollViewPrivate::mark_update()
	{
		if (ev_update)
			return;
		ev_update = add_event([this]() {
			update_scroll_view();
			ev_update = nullptr;
			return false;
		});
	}

	void cScrollViewPrivate::update_scroll_view()
	{
		float content_height = 0.f;
		float viewport_height = 0.f;
		if (e_content_container && e_content_viewport)
		{
			auto container_element = e_content_container->get_component<cElement>();
			auto viewport_element = e_content_viewport->get_component<cElement>();
			if (container_element && viewport_element)
			{
				content_height = container_element->ext.y;
				viewport_height = viewport_element->ext.y;
				if (content_height > 0.f && viewport_height > 0.f)
				{
					if (e_vertical_scroller && e_vertical_scroller_slider)
					{
						auto scroller_element = e_vertical_scroller->get_component<cElement>();
						auto slider_element = e_vertical_scroller_slider->get_component<cElement>();
						if (scroller_element && slider_element)
						{
							slider_element->set_h(scroller_element->ext.y * (viewport_height / content_height));
							slider_element->set_y(clamp(slider_element->pos.y, 0.f, scroller_element->ext.y - slider_element->ext.y));

							container_element->set_y(-container_element->ext.y * (slider_element->pos.y / scroller_element->ext.y));
						}
					}
				}
			}
		}
	}

	void cScrollViewPrivate::update_content_container()
	{
		if (e_content_container)
		{
			e_content_container->message_listeners.remove("scroll_view"_h);
			if (auto element = e_content_container->get_component<cElement>(); element)
				element->data_listeners.remove("scroll_view"_h);
		}
		e_content_container = World::instance()->root->find_with_file_id(content_container);
		if (e_content_container)
		{
			e_content_container->message_listeners.add([this](uint hash, void*, void*) {
				if (hash == "destroyed"_h)
					e_content_container = nullptr;
			});
			if (auto element = e_content_container->get_component<cElement>(); element)
			{
				element->data_listeners.add([this](uint hash) {
					if (hash == "ext"_h)
						mark_update();
				});
			}
		}
	}

	void cScrollViewPrivate::update_content_viewport()
	{
		if (e_content_viewport)
		{
			e_content_viewport->message_listeners.remove("scroll_view"_h);
			if (auto element = e_content_viewport->get_component<cElement>(); element)
				element->data_listeners.remove("scroll_view"_h);
		}
		e_content_viewport = World::instance()->root->find_with_file_id(content_viewport);
		if (e_content_viewport)
		{
			e_content_viewport->message_listeners.add([this](uint hash, void*, void*) {
				if (hash == "destroyed"_h)
					e_content_viewport = nullptr;
			});
			if (auto element = e_content_viewport->get_component<cElement>(); element)
			{
				element->data_listeners.add([this](uint hash) {
					if (hash == "ext"_h)
						mark_update();
				});
			}
		}
	}

	void cScrollViewPrivate::update_vertical_scroller()
	{
		if (e_vertical_scroller)
		{
			e_vertical_scroller->message_listeners.remove("scroll_view"_h);
			if (auto element = e_vertical_scroller->get_component<cElement>(); element)
				element->data_listeners.remove("scroll_view"_h);
		}
		e_vertical_scroller = World::instance()->root->find_with_file_id(vertical_scroller);
		if (e_vertical_scroller)
		{
			e_vertical_scroller->message_listeners.add([this](uint hash, void*, void*) {
				if (hash == "destroyed"_h)
					e_vertical_scroller = nullptr;
			});
			if (auto element = e_vertical_scroller->get_component<cElement>(); element)
			{
				element->data_listeners.add([this](uint hash) {
					if (hash == "ext"_h)
						mark_update();
				});
			}
		}
	}

	void cScrollViewPrivate::update_vertical_scroller_slider()
	{
		if (e_vertical_scroller_slider)
		{
			e_vertical_scroller_slider->message_listeners.remove("scroll_view"_h);
			if (auto element = e_vertical_scroller_slider->get_component<cElement>(); element)
				element->data_listeners.remove("scroll_view"_h);
			if (auto receiver = e_vertical_scroller_slider->get_component<cReceiver>(); receiver)
				receiver->event_listeners.remove("scroll_view"_h);
		}
		e_vertical_scroller_slider = World::instance()->root->find_with_file_id(vertical_scroller_slider);
		if (e_vertical_scroller_slider)
		{
			e_vertical_scroller_slider->message_listeners.add([this](uint hash, void*, void*) {
				if (hash == "destroyed"_h)
					e_vertical_scroller_slider = nullptr;
			});
			if (auto element = e_vertical_scroller_slider->get_component<cElement>(); element)
			{
				element->data_listeners.add([this](uint hash) {
					if (hash == "pos"_h || hash == "ext"_h)
						mark_update();
				});
			}
			if (auto receiver = e_vertical_scroller_slider->get_component<cReceiver>(); receiver)
			{
				receiver->event_listeners.add([this](uint type, const vec2& value) {
					if (type == "drag"_h)
					{
						auto slider_element = e_vertical_scroller_slider->get_component<cElement>();
						if (slider_element)
						{
							slider_element->add_pos(vec2(0.f, value.y));
							mark_update();
						}
					}
				});
			}
		}
	}

	void cScrollViewPrivate::set_content_container(const GUID& guid)
	{
		if (content_container == guid)
			return;
		content_container = guid;
		update_content_container();

		mark_update();
		data_changed("content_container"_h);
	}

	void cScrollViewPrivate::set_content_viewport(const GUID& guid)
	{
		if (content_viewport == guid)
			return;
		content_viewport = guid;
		update_content_viewport();

		mark_update();
		data_changed("content_viewport"_h);
	}

	void cScrollViewPrivate::set_vertical_scroller(const GUID& guid)
	{
		if (vertical_scroller == guid)
			return;
		vertical_scroller = guid;
		update_vertical_scroller();

		mark_update();
		data_changed("vertical_scroller"_h);
	}

	void cScrollViewPrivate::set_vertical_scroller_slider(const GUID& guid)
	{
		if (vertical_scroller_slider == guid)
			return;
		vertical_scroller_slider = guid;
		update_vertical_scroller_slider();

		mark_update();
		data_changed("vertical_scroller_slider"_h);
	}

	struct cScrollViewCreate : cScrollView::Create
	{
		cScrollViewPtr operator()(EntityPtr) override
		{
			return new cScrollViewPrivate();
		}
	}cScrollView_create;
	cScrollView::Create& cScrollView::create = cScrollView_create;
}
