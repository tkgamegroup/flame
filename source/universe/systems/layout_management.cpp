#include "../entity_private.h"
#include <flame/universe/world.h>
#include <flame/universe/systems/layout_management.h>
#include "../components/element_private.h"
#include <flame/universe/components/aligner.h>
#include "../components/layout_private.h"

namespace flame
{
	struct sLayoutManagementPrivate : sLayoutManagement
	{
		std::vector<cLayoutPrivate*> update_list;

		void add_to_update_list(cLayoutPrivate* l)
		{
			if (l->pending_update)
				return;
			update_list.push_back(l);
			l->pending_update = true;
			std::sort(update_list.begin(), update_list.end(), [](const auto& a, const auto& b) {
				return a->entity->depth_ < b->entity->depth_;
			});
		}

		void remove_from_update_list(cLayoutPrivate* l)
		{
			if (!l->pending_update)
				return;
			l->pending_update = false;
			for (auto it = update_list.begin(); it != update_list.end(); it++)
			{
				if ((*it) == l)
				{
					update_list.erase(it);
					return;
				}
			}
		}

		void calc_geometry(EntityPrivate* e)
		{
			if (!e->global_visible_)
				return;

			auto element = (cElementPrivate*)e->get_component(cElement);
			if (!element)
				return;

			element->calc_geometry();

			for (auto& c : e->children)
				calc_geometry(c.get());
		}

		void update() override
		{
			while (!update_list.empty())
			{
				auto l = update_list.back();
				update_list.erase(update_list.end() - 1);
				l->pending_update = false;
				if (l->entity->global_visible_)
					l->update();
			}

			calc_geometry((EntityPrivate*)world_->root());
		}
	};

	void sLayoutManagement::add_to_update_list(cLayout* l)
	{
		((sLayoutManagementPrivate*)this)->add_to_update_list((cLayoutPrivate*)l);
	}

	void sLayoutManagement::remove_from_update_list(cLayout* l)
	{
		((sLayoutManagementPrivate*)this)->remove_from_update_list((cLayoutPrivate*)l);
	}

	sLayoutManagement* sLayoutManagement::create()
	{
		return new sLayoutManagementPrivate;
	}

	void sLayoutManagement::destroy(sLayoutManagement* s)
	{
		delete (sLayoutManagementPrivate*)s;
	}
}
