//#include <flame/universe/entity.h>
//#include <flame/universe/world.h>
//#include "../components/element_private.h"
//#include "../components/text_private.h"
//#include <flame/universe/components/aligner.h>
//#include "../components/layout_private.h"

#include "../entity_private.h"
#include "../components/element_private.h"
#include "type_setting_private.h"

namespace flame
{
	void sTypeSettingBridge::add_to_sizing_list(sTypeSetting::AutoSizer* s, Entity* e)
	{
		((sTypeSettingPrivate*)this)->add_to_sizing_list(s, (EntityPrivate*)e);
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

		//while (!update_list.empty())
		//{
		//	auto l = update_list.back();
		//	update_list.erase(update_list.end() - 1);
		//	l->pending_update = false;
		//	if (l->entity->global_visibility)
		//		l->update();
		//}
	}

	//struct sLayoutManagementPrivate : sLayoutManagement
	//{
	//	std::vector<cLayoutPrivate*> update_list;


	//	void add_to_update_list(cLayoutPrivate* l)
	//	{
	//		if (l->pending_update)
	//			return;
	//		update_list.push_back(l);
	//		l->pending_update = true;
	//		std::sort(update_list.begin(), update_list.end(), [](const auto& a, const auto& b) {
	//			return a->entity->depth_ < b->entity->depth_;
	//		});
	//	}

	//	void remove_from_update_list(cLayoutPrivate* l)
	//	{
	//		if (!l->pending_update)
	//			return;
	//		l->pending_update = false;
	//		for (auto it = update_list.begin(); it != update_list.end(); it++)
	//		{
	//			if ((*it) == l)
	//			{
	//				update_list.erase(it);
	//				return;
	//			}
	//		}
	//	}

	//};

	//void sLayoutManagement::add_to_sizing_list(cText* t)
	//{
	//	((sLayoutManagementPrivate*)this)->add_to_sizing_list((cTextPrivate*)t);
	//}

	//void sLayoutManagement::remove_from_sizing_list(cText* t)
	//{
	//	((sLayoutManagementPrivate*)this)->remove_from_sizing_list((cTextPrivate*)t);
	//}

	//void sLayoutManagement::add_to_update_list(cLayout* l)
	//{
	//	((sLayoutManagementPrivate*)this)->add_to_update_list((cLayoutPrivate*)l);
	//}

	//void sLayoutManagement::remove_from_update_list(cLayout* l)
	//{
	//	((sLayoutManagementPrivate*)this)->remove_from_update_list((cLayoutPrivate*)l);
	//}

	sTypeSettingPrivate* sTypeSettingPrivate::create()
	{
		auto ret = _allocate(sizeof(sTypeSettingPrivate));
		new (ret) sTypeSettingPrivate;
		return (sTypeSettingPrivate*)ret;
	}

	sTypeSetting* sTypeSetting::create() { return sTypeSettingPrivate::create(); }
}
