#include <flame/universe/components/text.h>

namespace flame
{
	struct cTextPrivate : cText
	{
		std::wstring text;
		void* draw_cmd;

		cTextPrivate(graphics::FontAtlas* _font_atlas);
		cTextPrivate::~cTextPrivate();
		void draw(graphics::Canvas* canvas);
		void on_component_added(Component* c) override;
		Component* copy() override;
	};
}
