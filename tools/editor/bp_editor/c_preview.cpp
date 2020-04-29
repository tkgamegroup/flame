#include "bp_editor.h"

cPreview::cPreview() :
	Component("cPreview")
{
	auto e_page = utils::e_begin_docker_page(L"Preview", [](Capture&) {
		if (bp_editor.e_test)
			bp_editor.e_test->parent()->remove_child(bp_editor.e_test, false);
	}).second;
	{
		e_page->add_component(this);
	}
	
	utils::current_parent()->add_child(bp_editor.e_test);

	utils::e_end_docker_page();
}

cPreview::~cPreview()
{
	bp_editor.preview = nullptr;
}
