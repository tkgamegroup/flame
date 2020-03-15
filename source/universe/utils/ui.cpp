#include <flame/universe/utils/ui.h>

namespace flame
{
	namespace utils
	{
		static std::stack<graphics::FontAtlas*> _font_atlases;

		graphics::FontAtlas* current_font_atlas()
		{
			return _font_atlases.top();
		}

		void push_font_atlas(graphics::FontAtlas* font_atlas)
		{
			_font_atlases.push(font_atlas);
		}

		void pop_font_atlas()
		{
			_font_atlases.pop();
		}

		Vec2f next_element_pos = Vec2f(0.f);
		Vec2f next_element_size = Vec2f(0.f);
	}
}
