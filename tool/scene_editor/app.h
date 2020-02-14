#pragma once

#include <flame/utils/app.h>

namespace flame
{
	struct EnumInfo;
}

using namespace flame;
using namespace graphics;

struct MyApp : App
{
	void create();
};

extern MyApp app;

Entity* create_drag_edit(bool is_float);
void create_enum_combobox(EnumInfo* info, float width);
void create_enum_checkboxs(EnumInfo* info);
