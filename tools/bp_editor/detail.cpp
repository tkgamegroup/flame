#include "app.h"

cDetail::cDetail() :
	Component("cDetail")
{
	auto e_page = utils::e_begin_docker_page(L"Detail").second;
	{
		e_page->add_component(this);
	}

	utils::e_end_docker_page();
}

cDetail::~cDetail()
{
	app.detail = nullptr;
}
