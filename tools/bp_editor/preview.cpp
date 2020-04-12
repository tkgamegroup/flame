#include "app.h"

cPreview::cPreview() :
	Component("cPreview")
{
	auto e_page = utils::e_begin_docker_page(L"Preview").second;
	{
		e_page->add_component(this);
	}

	utils::e_end_docker_page();
}

cPreview::~cPreview()
{
	app.preview = nullptr;
}
