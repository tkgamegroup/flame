#include "bp_editor.h"

cPreview::cPreview() :
	Component("cPreview")
{
	auto& ui = bp_editor.window->ui;

	auto e_page = ui.e_begin_docker_page(L"Preview", [](Capture&) {
		if (bp_editor.e_test)
			bp_editor.e_test->parent->remove_child(bp_editor.e_test, false);
	}).second;
	{
		e_page->add_component(this);
	}
	
	ui.parents.top()->add_child(bp_editor.e_test);

	ui.e_end_docker_page();
}

cPreview::~cPreview()
{
	bp_editor.preview = nullptr;
}
