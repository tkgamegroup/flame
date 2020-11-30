#include "../world_private.h"
#include "../components/element_private.h"
#include "../components/aligner_private.h"
#include "../components/layout_private.h"
#include "layout_system_private.h"

namespace flame
{
	void sLayoutSystemPrivate::add_to_sizing_list(cElementPrivate* e)
	{
		if (e->pending_sizing)
			return;
		e->pending_sizing = true;
		auto it = sizing_list.begin();
		for (; it != sizing_list.end(); it++)
		{
			if ((*it)->entity->depth < e->entity->depth)
				break;
		}
		sizing_list.emplace(it, e);
	}

	void sLayoutSystemPrivate::remove_from_sizing_list(cElementPrivate* e)
	{
		if (!e->pending_sizing)
			return;
		e->pending_sizing = false;
		std::erase_if(sizing_list, [&](const auto& i) {
			return i == e;
		});
	}

	void sLayoutSystemPrivate::add_to_layouting_list(cLayoutPrivate* l)
	{
		if (l->pending_layouting)
			return;
		l->pending_layouting = true;
		layouting_list.push_back(l);
		auto it = layouting_list.begin();
		for (; it != layouting_list.end(); it++)
		{
			if ((*it)->entity->depth < l->entity->depth)
				break;
		}
		layouting_list.emplace(it, l);
	}

	void sLayoutSystemPrivate::remove_from_layouting_list(cLayoutPrivate* l)
	{
		if (!l->pending_layouting)
			return;
		l->pending_layouting = false;
		std::erase_if(layouting_list, [&](const auto& i) {
			return i == l;
		});
	}

	void sLayoutSystemPrivate::on_added()
	{
		window = (Window*)world->find_object("flame::Window");
	}

	void sLayoutSystemPrivate::update()
	{
		if (window)
		{
			auto element = world->root->get_component_t<cElementPrivate>();
			auto size = window->get_size();
			element->set_width(size.x);
			element->set_height(size.y);
		}

		while (!sizing_list.empty())
		{
			auto e = sizing_list.front();
			e->pending_sizing = false;
			if (e->entity->global_visibility)
			{
				auto size = vec2(-1.f);
				for (auto& m : e->measurables)
				{
					vec2 r;
					m.second(m.first, r);
					size = max(size, r);
				}

				auto w = 0.f, h = 0.f;
				if (size.x >= 0.f)
				{
					w = size.x + e->padding_size[0];
					e->set_width(w);
				}
				if (size.y >= 0.f)
				{
					h = size.y + e->padding_size[1];
					e->set_height(h);
				}

				auto aligner = e->entity->get_component_t<cAlignerPrivate>();
				if (aligner)
					aligner->desired_size = vec2(w, h);
			}
			sizing_list.pop_front();
		}

		while (!layouting_list.empty())
		{
			auto l = layouting_list.front();
			l->pending_layouting = false;
			if (l->entity->global_visibility)
				l->update();
			layouting_list.pop_front();
		}
	}

	sLayoutSystem* sLayoutSystem::create()
	{
		return f_new<sLayoutSystemPrivate>();
	}
}
