#include <flame/universe/ui/utils.h>

namespace flame
{
	namespace ui
	{
		static std::stack<graphics::FontAtlas*> _font_atlases;
		static std::stack<Entity*> _parents;
		static Entity* _current_entity;

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

		Entity* current_entity()
		{
			return _current_entity;
		}

		void set_current_entity(Entity* e)
		{
			_current_entity = e;
		}

		Entity* current_parent()
		{
			return _parents.top();
		}

		void push_parent(Entity* parent)
		{
			_parents.push(parent);
		}

		void pop_parent()
		{
			_parents.pop();
		}
	}
}
