#include <flame/universe/systems/timer_management.h>
#include "../components/timer_private.h"

namespace flame
{
	struct sTimerManagementPrivate : sTimerManagement
	{
		std::vector<cTimerPrivate*> update_list;
		bool updating;
		bool need_clear;

		sTimerManagementPrivate() :
			updating(false),
			need_clear(false)
		{
		}

		void add_to_update_list(cTimerPrivate* t)
		{
			if (t->timing)
				return;
			update_list.push_back(t);
			t->timing = true;
			std::sort(update_list.begin(), update_list.end(), [](const auto& a, const auto& b) {
				return a->entity->depth_ < b->entity->depth_ || a->entity->index_ < b->entity->index_;
			});
		}

		void remove_from_update_list(cTimerPrivate* t)
		{
			if (!t->timing)
				return;
			t->timing = false;
			for (auto it = update_list.begin(); it != update_list.end(); it++)
			{
				if ((*it) == t)
				{
					if (!updating)
						update_list.erase(it);
					else
					{
						*it = nullptr;
						need_clear = true;
					}
					return;
				}
			}
		}

		void update() override
		{
			auto dt = looper().delta_time;
			updating = true;
			for (auto& t : update_list)
			{
				t->_t += dt;
				if (t->_t >= t->interval)
				{
					t->_t = 0;
					t->callbacks.call();
				}
			}
			if (need_clear)
			{
				for (auto it = update_list.begin(); it != update_list.end();)
				{
					if ((*it) == nullptr)
						it = update_list.erase(it);
					else
						it++;
				}
			}
			updating = false;
		}
	};

	void sTimerManagement::add_to_update_list(cTimer* t)
	{
		((sTimerManagementPrivate*)this)->add_to_update_list((cTimerPrivate*)t);
	}

	void sTimerManagement::remove_from_update_list(cTimer* t)
	{
		((sTimerManagementPrivate*)this)->remove_from_update_list((cTimerPrivate*)t);
	}

	sTimerManagement* sTimerManagement::create()
	{
		return new sTimerManagementPrivate;
	}
}
