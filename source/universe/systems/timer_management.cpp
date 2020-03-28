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
			if (t->updating)
				return;
			update_list.push_back(t);
			t->updating = true;
			std::sort(update_list.begin(), update_list.end(), [](const auto& a, const auto& b) {
				if (!a || !b)
					return true;
				return a->entity->depth_ < b->entity->depth_ && a->entity->index_ < b->entity->index_;
			});
		}

		void remove_from_update_list(cTimerPrivate* t)
		{
			if (!t->updating)
				return;
			t->updating = false;
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
				auto do_remove = false;

				if (!t->entity->global_visible_)
					do_remove = true;
				else
				{
					auto do_call = false;

					t->_time += dt;
					t->_total_time += dt;
					if (t->_time >= t->interval)
					{
						t->_time = 0;
						t->_times++;
						t->callback->call();
					}
					if (t->max_time > 0.f && t->_total_time >= t->max_time)
					{
						t->_total_time = t->max_time;
						do_remove = true;
					}
					if (t->max_times > 0 && t->_times >= t->max_times)
					{
						t->_times = t->max_times;
						do_remove = true;
					}
				}

				if (do_remove)
				{
					t->updating = false;
					t->reset();
					t = nullptr;
					need_clear = true;
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
