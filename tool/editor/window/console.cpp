#include <flame/universe/ui/utils.h>

#include "../app.h"
#include "console.h"

struct cConsolePrivate : cConsole
{
	std::unique_ptr<Closure<void(void* c, const std::wstring & cmd, cConsole * console)>> cmd_callback;
	std::unique_ptr<Closure<void(void* c)>> close_callback;

	~cConsolePrivate()
	{
		close_callback->call();
	}
};

void cConsole::print(const std::wstring& str)
{
	c_text_log->set_text((c_text_log->text() + str + L"\n").c_str());
}

Entity* open_console(void (*cmd_callback)(void* c, const std::wstring& cmd, cConsole* console), const Mail<>& cmd_callback_capture, void (*close_callback)(void* c), const Mail<>& close_callback_capture, const std::wstring& init_str, const Vec2f& pos)
{
	ui::push_parent(app.root);
	ui::e_begin_docker_container(pos, Vec2f(421.f, 323.f));
	ui::e_begin_docker();
	auto e_page = ui::e_begin_docker_page(L"Console").page;
	{
		e_page->get_component(cElement)->inner_padding_ = Vec4f(8.f);
		auto c_layout = ui::c_layout(LayoutVertical);
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
	}
	auto c_console = new_u_object<cConsolePrivate>();
	{
		auto c = new Closure<void(void* c, const std::wstring & cmd, cConsole * console)>;
		c->function = cmd_callback;
		c->capture = cmd_callback_capture;
		c_console->cmd_callback.reset(c);
	}
	{
		auto c = new Closure<void(void* c)>;
		c->function = close_callback;
		c->capture = close_callback_capture;
		c_console->close_callback.reset(c);
	}
	e_page->add_component(c_console);

	ui::e_begin_scroll_view1(ScrollbarVertical, Vec2f(0.f));
	ui::e_begin_layout(0.f, 0.f, LayoutVertical)->get_component(cElement)->clip_children = true;
	ui::c_aligner(SizeFitParent, SizeFitParent);
	c_console->c_text_log = ui::e_text(!init_str.empty() ? (init_str + L"\n").c_str() : L"")->get_component(cText);
	ui::e_end_layout();
	ui::e_end_scroll_view1(ui::style(ui::FontSize).u()[0]);

	ui::e_button(L"Clear", [](void* c, Entity*) {
		auto text = *(cText**)c;
		text->set_text(L"");
	}, new_mail_p(c_console->c_text_log));

	ui::e_begin_layout(0.f, 0.f, LayoutHorizontal, 4.f);
	ui::c_aligner(SizeFitParent, SizeFixed);
	c_console->c_edit_input = ui::e_edit(0.f)->get_component(cEdit);
	ui::e_button(L"Exec", [](void* c, Entity*) {
		auto c_console = *(cConsolePrivate**)c;
		auto input_text = c_console->c_edit_input->text;
		c_console->print(input_text->text());
		c_console->cmd_callback->call(input_text->text(), c_console);

		input_text->set_text(L"");
		c_console->c_edit_input->cursor = 0;
	}, new_mail_p(c_console));
	ui::e_end_layout();

	ui::e_end_docker_page();
	ui::e_end_docker();
	ui::e_end_docker_container();
	ui::pop_parent();

	return e_page;
}
