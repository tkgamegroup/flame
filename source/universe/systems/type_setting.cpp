//#include <flame/universe/entity.h>
//#include <flame/universe/world.h>
//#include "../components/element_private.h"
//#include "../components/text_private.h"
//#include <flame/universe/components/aligner.h>

#include "../entity_private.h"
#include "../components/element_private.h"
#include "../components/layout_private.h"
#include "type_setting_private.h"

namespace flame
{
	void sTypeSettingBridge::add_to_sizing_list(sTypeSetting::AutoSizer* s, Entity* e)
	{
		((sTypeSettingPrivate*)this)->add_to_sizing_list(s, (EntityPrivate*)e);
	}

	void sTypeSettingBridge::add_to_layouting_list(cLayout* l)
	{
		((sTypeSettingPrivate*)this)->add_to_layouting_list((cLayoutPrivate*)l);
	}

	void sTypeSettingBridge::remove_from_layouting_list(cLayout* l)
	{
		((sTypeSettingPrivate*)this)->remove_from_layouting_list((cLayoutPrivate*)l);
	}

	void sTypeSettingPrivate::add_to_sizing_list(sTypeSetting::AutoSizer* s, EntityPrivate* e)
	{
		if (s->pending_sizing)
			return;
		auto it = sizing_list.begin();
		for (; it != sizing_list.end(); it++)
		{
			if (std::get<1>(*it)->depth < e->depth)
				break;
		}
		sizing_list.emplace(it, s, e, (cElementPrivate*)e->get_component(cElement::type_hash));
		s->pending_sizing = true;
	}

	void sTypeSettingPrivate::remove_from_sizing_list(sTypeSetting::AutoSizer* s)
	{
		if (!s->pending_sizing)
			return;
		s->pending_sizing = false;
		std::erase_if(sizing_list, [&](const auto& i) {
			return std::get<0>(i) == s;
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

	void sTypeSettingPrivate::update()
	{
		while (!sizing_list.empty())
		{
			auto& s = sizing_list.front();
			std::get<0>(s)->pending_sizing = false;
			if (std::get<1>(s)->global_visibility)
			{
				auto size = std::get<0>(s)->measure();
				std::get<2>(s)->set_width(size.x());
				std::get<2>(s)->set_height(size.y());
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

	sTypeSettingPrivate* sTypeSettingPrivate::create()
	{
		auto ret = _allocate(sizeof(sTypeSettingPrivate));
		new (ret) sTypeSettingPrivate;
		return (sTypeSettingPrivate*)ret;
	}

	sTypeSetting* sTypeSetting::create() { return sTypeSettingPrivate::create(); }
}
