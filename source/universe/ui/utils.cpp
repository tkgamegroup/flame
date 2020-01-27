#include <flame/universe/ui/utils.h>

namespace flame
{
	namespace ui
	{
		static std::stack<graphics::FontAtlas*> _font_atlases;
		static Entity* _current_entity;
		static std::stack<Entity*> _parents;
		static Entity* _current_root;

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

		Entity* current_root()
		{
			return _current_root;
		}

		void set_current_root(Entity* e)
		{
			_current_root = e;
		}

		Entity* next_entity = nullptr;
		Vec2f next_element_pos = Vec2f(0.f);
		Vec2f next_element_size = Vec2f(0.f);
	}
}
