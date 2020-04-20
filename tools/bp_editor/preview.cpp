#include "app.h"

cPreview::cPreview() :
	Component("cPreview")
{
	auto e_page = utils::e_begin_docker_page(L"Preview", [](void*) {
		app.e_test->parent()->remove_child(app.e_test, false);
	}).second;
	{
		e_page->add_component(this);
	}
	
	utils::current_parent()->add_child(app.e_test);

	utils::e_end_docker_page();
}

cPreview::~cPreview()
{
	app.preview = nullptr;
}
