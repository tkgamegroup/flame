#pragma once

#include "app.h"

struct DebuggerView : View
{
	DebuggerView();
	DebuggerView(const std::string& name);
	~DebuggerView();
	void on_draw() override;
};

struct DebuggerWindow : Window
{
	DebuggerWindow();
	void open_view(bool new_instance) override;
	void open_view(const std::string& name) override;
};

extern DebuggerWindow debugger_window;
