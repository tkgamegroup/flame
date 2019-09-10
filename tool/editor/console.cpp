#include <flame/graphics/font.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/window.h>

#include "console.h"

struct cConsolePrivate : cConsole
{
	std::unique_ptr<Closure<void(void* c, const std::wstring & cmd, cConsole * console)>> processor;
};

void cConsole::print(const std::wstring& str)
{
	c_text_log->set_text(c_text_log->text() + str + L"\n");
}

void cConsole::update()
{
}

Entity* open_console(void (*callback)(void* c, const std::wstring& cmd, cConsole* console), const Mail<>& capture, const Vec2f& pos, graphics::FontAtlas* font_atlas)
{
	auto e_container = get_docker_container_model()->copy();
	{
		auto c_element = (cElement*)e_container->find_component(cH("Element"));
		c_element->x = pos.x();
		c_element->y = pos.y();
		c_element->width = 400.f;
		c_element->height = 300.f;
	}

	auto e_docker = get_docker_model()->copy();
	e_container->add_child(e_docker);
	auto e_tabbar = e_docker->child(0);
	auto e_pages = e_docker->child(1);

	auto e_tab = get_docker_tab_model()->copy();
	((cText*)e_tab->find_component(cH("Text")))->set_text(L"Console");
	e_tabbar->add_child(e_tab);

	auto e_page = get_docker_page_model()->copy();
	e_pages->add_child(e_page);
	{
		((cElement*)e_page->find_component(cH("Element")))->inner_padding = Vec4f(8.f);

		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_page->add_component(c_layout);
	}

	auto e_log_view = Entity::create();
	{
		auto c_element = cElement::create();
		c_element->clip_children = true;
		e_log_view->add_component(c_element);

		auto c_aligner = cAligner::create();
		c_aligner->width_policy = SizeFitParent;
		c_aligner->height_policy = SizeFitParent;
		e_log_view->add_component(c_aligner);

		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_log_view->add_component(c_layout);
	}

	auto e_log = Entity::create();
	e_log_view->add_child(e_log);
	{
		e_log->add_component(cElement::create());

		e_log->add_component(cText::create(font_atlas));
	}

	e_page->add_child(wrap_standard_scrollbar(e_log_view, ScrollbarVertical, true, font_atlas->pixel_height));

	auto e_btn_clear = create_standard_button(font_atlas, 1.f, L"Clear");
	e_page->add_child(e_btn_clear);
	((cEventReceiver*)e_btn_clear->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
		if (is_mouse_clicked(action, key))
		{
			auto text = *(cText**)c;
			text->set_text(L"");
		}
	}, new_mail_p(e_log->find_component(cH("Text"))));

	auto e_input = Entity::create();
	e_page->add_child(e_input);
	{
		e_input->add_component(cElement::create());

		auto c_aligner = cAligner::create();
		c_aligner->width_policy = SizeFitParent;
		e_input->add_component(c_aligner);

		auto c_layout = cLayout::create();
		c_layout->type = LayoutHorizontal;
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		e_input->add_component(c_layout);
	}

	auto e_input_edit = create_standard_edit(0.f, font_atlas, 1.f);
	e_input->add_child(e_input_edit);

	auto e_btn_exec = create_standard_button(font_atlas, 1.f, L"Exec");
	e_input->add_child(e_btn_exec);

	auto c_console = new_component<cConsolePrivate>();
	c_console->c_text_log = (cText*)e_log->find_component(cH("Text"));
	c_console->c_edit_input = (cEdit*)e_input_edit->find_component(cH("Edit"));
	{
		auto c = new Closure<void(void* c, const std::wstring & cmd, cConsole * console)>;
		c->function = callback;
		c->capture = capture;
		c_console->processor.reset(c);
	}
	((cEventReceiver*)e_btn_exec->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
		if (is_mouse_clicked(action, key))
		{
			auto c_console = *(cConsolePrivate**)c;
			auto input_text = c_console->c_edit_input->text;
			c_console->print(input_text->text());
			auto p = c_console->processor.get();
			p->function(p->capture.p, input_text->text(), c_console);

			input_text->set_text(L"");
			c_console->c_edit_input->cursor = 0;
		}
	}, new_mail_p(c_console));
	e_container->add_component(c_console);

	return e_container;
}
