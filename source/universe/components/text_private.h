#include <flame/universe/components/text.h>

namespace flame
{
	struct cTextPrivate : cText
	{
		void* draw_cmd;
		bool peeding_sizing;

		cTextPrivate();
		cTextPrivate::~cTextPrivate();
		void draw(graphics::Canvas* canvas);
		void set_size_auto();
		void on_component_added(Component* c) override;
	};
}
