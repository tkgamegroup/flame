#pragma once

#include <flame/engine/ui/window.h>

struct TextEditor : flame::ui::Window
{
	TextEditor();
	~TextEditor();
	virtual void on_show() override;
};

extern TextEditor *text_editor;
