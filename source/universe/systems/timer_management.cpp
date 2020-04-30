#include <flame/universe/systems/timer_management.h>
#include "../components/timer_private.h"

namespace flame
{
	struct sTimerManagementPrivate : sTimerManagement
	{
		std::vector<cTimerPrivate*> update_list;
		bool updating;
		bool need_clean;

		sTimerManagementPrivate() :
			updating(false),
			need_clean(false)
		{
		}

		void add_to_update_list(cTimerPrivate* t)
		{
			if (t->_time > 0.f)
				return;
			update_list.push_back(t);
			t->_time = 0.f;
			std::sort(update_list.begin(), update_list.end(), [](const auto& a, const auto& b) {
				if (!a || !b)
					return true;
				return a->entity->depth_ < b->entity->depth_ && a->entity->index_ < b->entity->index_;
			});
		}

		void remove_from_update_list(cTimerPrivate* t)
		{
			if (t->_time < 0.f)
				return;
			t->_time = -1;
			t->reset();
			for (auto it = update_list.begin(); it != update_list.end(); it++)
			{
				if ((*it) == t)
				{
					if (!updating)
						update_list.erase(it);
					else
					{
						*it = nullptr;
						need_clean = true;
					}
					return;
				}
			}
		}

		void update() override
		{
			auto dt = looper().delta_time;
			updating = true;
			for (auto t : update_list)
			{
				if (!t)
					continue;
				t->_time += dt;
				if (t->_time >= t->interval)
				{
					t->_time = 0;
					t->callback->c._current = t;
					t->callback->call();
				}
			}
			if (need_clean)
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
