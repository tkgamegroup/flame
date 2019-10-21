#include <flame/universe/systems/layout_management.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/aligner.h>

namespace flame
{
	struct sLayoutManagementPrivate : sLayoutManagement
	{
		std::vector<cLayout*> update_list;

		void add_to_update_list(cLayout* l)
		{
			update_list.push_back(l);
			std::sort(update_list.begin(), update_list.end(), [](const cLayout* a, const cLayout* b) {
				return a->entity->order_ < b->entity->order_;
			});
		}

		void update(Entity* root) override
		{

		}
	};

	void sLayoutManagement::add_to_update_list(cLayout* l)
	{
		((sLayoutManagementPrivate*)this)->add_to_update_list(l);
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
