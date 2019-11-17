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
	struct Entity;
	struct cElement;
	struct cText;
	struct cCombobox;
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
	BP* canvas_bp;
	Canvas* canvas;

	FontAtlas* font_atlas_pixel;

	uint fps;

	Universe* u;
	Entity* root;
	cElement* c_element_root;
	cText* c_text_fps;

	std::vector<TypeinfoDatabase*> dbs;

	void create();
	void run();
};

extern App app;

Entity* create_drag_edit(FontAtlas* font_atlas, float font_size_scale, bool is_float);

void create_enum_combobox(EnumInfo* info, float width, FontAtlas* font_atlas, float font_size_scale, Entity* parent);
void create_enum_checkboxs(EnumInfo* info, FontAtlas* font_atlas, float font_size_scale, Entity* parent);

Entity* popup_dialog(Entity* e);
void popup_message_dialog(Entity* e, const std::wstring& text);
void popup_confirm_dialog(Entity* e, const std::wstring& title, void (*callback)(void* c, bool yes), const Mail<>& capture);
void popup_input_dialog(Entity* e, const std::wstring& title, void (*callback)(void* c, bool ok, const std::wstring& text), const Mail<>& capture);
