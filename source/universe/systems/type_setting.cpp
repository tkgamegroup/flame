#include "../world_private.h"
#include "../components/element_private.h"
#include "../components/aligner_private.h"
#include "../components/layout_private.h"
#include "type_setting_private.h"

namespace flame
{
	void sTypeSettingPrivate::add_to_sizing_list(sTypeSetting::AutoSizer* s, EntityPrivate* e)
	{
		if (s->pending_sizing)
			return;
		auto it = sizing_list.begin();
		for (; it != sizing_list.end(); it++)
		{
			if (it->second->depth < e->depth)
				break;
		}
		sizing_list.emplace(it, s, e);
		s->pending_sizing = true;
	}

	void sTypeSettingPrivate::remove_from_sizing_list(sTypeSetting::AutoSizer* s)
	{
		if (!s->pending_sizing)
			return;
		s->pending_sizing = false;
		std::erase_if(sizing_list, [&](const auto& i) {
			return i.first == s;
		});
	}

	void sTypeSettingPrivate::add_to_layouting_list(cLayoutPrivate* l)
	{
		if (l->pending_layouting)
			return;
		l->pending_layouting = true;
		layouting_list.push_back(l);
		auto it = layouting_list.begin();
		for (; it != layouting_list.end(); it++)
		{
			if (((EntityPrivate*)(*it)->entity)->depth < ((EntityPrivate*)l->entity)->depth)
				break;
		}
		layouting_list.emplace(it, l);
	}

	void sTypeSettingPrivate::remove_from_layouting_list(cLayoutPrivate* l)
	{
		if (!l->pending_layouting)
			return;
		l->pending_layouting = false;
		std::erase_if(layouting_list, [&](const auto& i) {
			return i == l;
		});
	}

	void sTypeSettingPrivate::on_added()
	{
		window = (Window*)((WorldPrivate*)world)->find_object("flame::Window");
	}

	void sTypeSettingPrivate::update()
	{
		if (window)
		{
			auto element = (cElementPrivate*)((WorldPrivate*)world)->root->get_component(cElement::type_hash);
			auto size = window->get_size();
			element->set_width(size.x());
			element->set_height(size.y());
		}

		while (!sizing_list.empty())
		{
			auto& s = sizing_list.front();
			s.first->pending_sizing = false;
			if (std::get<1>(s)->global_visibility)
			{
				auto element = (cElementPrivate*)s.second->get_component(cElement::type_hash);
				auto aligner = (cAlignerPrivate*)s.second->get_component(cAligner::type_hash);
				auto size = s.first->measure();
				auto w = 0.f, h = 0.f;
				if (s.first->auto_width)
				{
					w = size.x() + element->padding[0] + element->padding[2];
					element->set_width(w);
				}
				if (s.first->auto_height)
				{
					h = size.y() + element->padding[1] + element->padding[3];
					element->set_height(h);
				}
				if (aligner)
					aligner->desired_size = Vec2f(w, h);
			}
			sizing_list.pop_front();
		}

		while (!layouting_list.empty())
		{
			auto l = layouting_list.front();
			l->pending_layouting = false;
			if (((EntityPrivate*)l->entity)->global_visibility)
				l->update();
			layouting_list.pop_front();
		}
	}

	sTypeSetting* sTypeSetting::create()
	{
		return f_new<sTypeSettingPrivate>();
	}
}
