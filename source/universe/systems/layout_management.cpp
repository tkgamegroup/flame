#include <flame/universe/systems/layout_management.h>
#include "../components/layout_private.h"
#include <flame/universe/components/aligner.h>

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
			std::sort(update_list.begin(), update_list.end(), [](const cLayout* a, const cLayout* b) {
				return a->entity->order_ < b->entity->order_;
			});
		}

		void update(Entity* root) override
		{
			while (!update_list.empty())
			{
				auto l = update_list.back();
				update_list.erase(update_list.end() - 1);
				l->pending_update = false;
				l->update();
			}
		}
	};

	void sLayoutManagement::add_to_update_list(cLayout* l)
	{
		((sLayoutManagementPrivate*)this)->add_to_update_list((cLayoutPrivate*)l);
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
