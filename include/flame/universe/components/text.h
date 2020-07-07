#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;

	struct FLAME_R(cText : Component, all)
	{
		cText() :
			Component("cText")
		{
		}

		//FLAME_RV(graphics::FontAtlas*, font_atlas);
		//FLAME_RV(StringW, text);
		//FLAME_RV(uint, font_size);
		//FLAME_RV(Vec4c, color);
		//FLAME_RV(bool, auto_size);


		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_text)(const wchar_t* text/* nullptr means no check (you need to set by yourself) */, int length = -1,  void* sender = nullptr);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_font_size)(uint s, void* sender = nullptr);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_color)(const Vec4c& c, void* sender = nullptr);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_auto_size)(bool v, void* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS static cText* create();
	};
}
