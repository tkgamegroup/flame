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

	virtual void update() override;
};

void open_console(void (*callback)(void* c, const std::wstring& cmd, cConsole* console), const Mail<>& capture, const std::wstring& init_str, const Vec2f& pos);
