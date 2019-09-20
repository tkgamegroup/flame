#pragma once

#include <flame/foundation/foundation.h>

namespace flame
{
	struct Window;

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

	struct Entity;
	struct cElement;
	struct cText;
}

using namespace flame;
using namespace graphics;

struct App
{
	Window* w;
	Device* d;
	Semaphore* render_finished;
	SwapchainResizable* scr;
	Fence* fence;
	std::vector<Commandbuffer*> sc_cbs;
	std::vector<Commandbuffer*> extra_cbs;

	FontAtlas* font_atlas_pixel;
	FontAtlas* font_atlas_sdf;
	Canvas* canvas;
	int rt_frame;

	Entity* root;
	cElement* c_element_root;
	cText* c_text_fps;

	void create();
	void run();
};

extern App app;

Entity* create_enum_combobox(EnumInfo* info, float width, FontAtlas* font_atlas, float sdf_scale);

