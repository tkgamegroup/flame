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
		void auto_set_size();
		void on_component_added(Component* c) override;
		void on_visibility_changed() override;
	};
}
