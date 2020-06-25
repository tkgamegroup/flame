#include <flame/universe/entity.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/layout_management.h>
#include "../components/element_private.h"
#include "../components/text_private.h"
#include <flame/universe/components/aligner.h>
#include "../components/layout_private.h"

namespace flame
{
	struct sLayoutManagementPrivate : sLayoutManagement
	{
		std::vector<cTextPrivate*> sizing_list;
		std::vector<cLayoutPrivate*> update_list;

		void add_to_sizing_list(cTextPrivate* t)
		{
			if (t->pending_sizing)
				return;
			sizing_list.push_back(t);
			t->pending_sizing = true;
			std::sort(sizing_list.begin(), sizing_list.end(), [](const auto& a, const auto& b) {
				return a->entity->depth_ < b->entity->depth_;
			});
		}

		void remove_from_sizing_list(cTextPrivate* t)
		{
			if (!t->pending_sizing)
				return;
			t->pending_sizing = false;
			for (auto it = sizing_list.begin(); it != sizing_list.end(); it++)
			{
				if ((*it) == t)
				{
					sizing_list.erase(it);
					return;
				}
			}
		}

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

		void calc_geometry(Entity* e)
		{
			if (!e->global_visibility)
				return;

			auto element = (cElementPrivate*)e->get_component(cElement);
			if (!element)
				return;

			element->calc_geometry();

			for (auto c : e->children)
				calc_geometry(c);
		}

		void update() override
		{
			while (!sizing_list.empty())
			{
				auto e = sizing_list.back();
				sizing_list.erase(sizing_list.end() - 1);
				e->pending_sizing = false;
				auto ee = e->entity;
				if (ee->global_visibility)
				{
					auto t = (cTextPrivate*)ee->get_component(cText);
					if (t)
						t->auto_set_size();
				}
			}

			while (!update_list.empty())
			{
				auto l = update_list.back();
				update_list.erase(update_list.end() - 1);
				l->pending_update = false;
				if (l->entity->global_visibility)
					l->update();
			}

			calc_geometry(world_->root);
		}
	};

	void sLayoutManagement::add_to_sizing_list(cText* t)
	{
		((sLayoutManagementPrivate*)this)->add_to_sizing_list((cTextPrivate*)t);
	}

	void sLayoutManagement::remove_from_sizing_list(cText* t)
	{
		((sLayoutManagementPrivate*)this)->remove_from_sizing_list((cTextPrivate*)t);
	}

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
