#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cText;
	struct cEdit;
}

using namespace flame;

struct cConsole : Component
{
	cText* c_text_log;
	cEdit* c_edit_input;

	cConsole() :
		Component("Console")
	{
	}

	void print(const std::wstring& str);
};

Entity* open_console(void (*cmd_callback)(void* c, const std::wstring& cmd, cConsole* console), const Mail<>& cmd_callback_capture, void (*close_callback)(void* c), const Mail<>& close_callback_capture, const std::wstring& init_str, const Vec2f& pos);
