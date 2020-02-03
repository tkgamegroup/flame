#pragma once

#include <flame/foundation/foundation.h>

namespace flame
{
	struct BP;
	struct Window;
	struct EnumInfo;

	namespace graphics
	{
		struct Device;
		struct Semaphore;
		struct SwapchainResizable;
		struct Fence;
		struct Commandbuffer;
		struct FontAtlas;
		struct Canvas;
	}

	struct Universe;
	struct s2DRenderer;
	struct Entity;
	struct cElement;
	struct cText;
	struct cCombobox;
}

using namespace flame;
using namespace graphics;

struct App
{
	SysWindow* w;
	Device* d;
	SwapchainResizable* scr;
	Fence* fence;
	Array<Commandbuffer*> sc_cbs;
	Semaphore* render_finished;
	std::vector<Commandbuffer*> extra_cbs;

	FontAtlas* font_atlas_pixel;

	Universe* u;
	s2DRenderer* s_2d_renderer;
	Entity* root;
	cElement* c_element_root;

	void create();
	void run();
};

extern App app;

Entity* create_drag_edit(bool is_float);
void create_enum_combobox(EnumInfo* info, float width);
void create_enum_checkboxs(EnumInfo* info);
