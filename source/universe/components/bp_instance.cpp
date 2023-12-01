
#include "../entity_private.h"
#include "bp_instance_private.h"
#include "../blueprint_library/library.h"

namespace flame
{
	cBpInstancePrivate::~cBpInstancePrivate()
	{
		if (bp_ins && !bp_ins->is_static)
			delete bp_ins;
		if (bp)
			Blueprint::release(bp);
	}

	void cBpInstancePrivate::set_bp_name(const std::filesystem::path& name)
	{
		if (bp_name == name)
			return;
		bp_name = name;

		if (bp_ins)
		{
			if (!bp_ins->is_static)
				delete bp_ins;
			bp_ins = nullptr;
		}
		if (bp)
		{
			Blueprint::release(bp);
			bp = nullptr;
		}

		bp = Blueprint::get(bp_name);
		if (bp)
		{
			if (bp->is_static)
				bp_ins = BlueprintInstance::get(bp->name_hash);
			else
				bp_ins = BlueprintInstance::create(bp);
			bp_ins->set_variable("self"_h, entity);
		}
	}

	static bool run_coroutine(BlueprintInstanceGroup* g)
	{
		while (true)
		{
			if (g->wait_time != 0.f)
			{
				if (g->wait_time == -1.f)
					g->wait_time = 0.f;
				else
				{
					g->wait_time -= delta_time;
					if (g->wait_time <= 0.f)
						g->wait_time = 0.f;
				}
				return true;
			}
			if (!g->instance->step(g))
				return false;
		}
		return false;
	}

	void cBpInstancePrivate::start_coroutine(BlueprintInstanceGroup* group, float delay)
	{
		assert(group->instance == bp_ins);
		assert(group->executiona_type == BlueprintExecutionCoroutine);

		for (auto g : coroutines)
		{
			if (g == group)
			{
				printf("start_coroutine: coroutine already existed in list\n");
				return;
			}
		}

		if (executing_coroutines)
		{
			peeding_add_coroutines.push_back(group);
			return;
		}

		bp_ins->prepare_executing(group);
		group->wait_time = delay;
		if (run_coroutine(group))
			coroutines.push_back(group);
	}

	void cBpInstancePrivate::start()
	{
		if (bp_ins)
		{
			if (auto g = bp_ins->get_group("start"_h); g)
			{
				if (g->executiona_type == BlueprintExecutionCoroutine)
					start_coroutine(g);
				else
				{
					bp_ins->prepare_executing(g);
					bp_ins->run(g);
				}
			}
			if (auto g = bp_ins->get_group("update"_h); g)
			{
				assert(g->executiona_type == BlueprintExecutionFunction);
				update_group = g;
			}
		}
	}

	void cBpInstancePrivate::update()
	{
		if (update_group)
		{
			bp_ins->prepare_executing(update_group);
			bp_ins->run(update_group);
		}

		executing_coroutines = true;
		for (auto it = coroutines.begin(); it != coroutines.end();)
		{
			auto g = *it;
			if (!run_coroutine(g))
				it = coroutines.erase(it);
			else
				it++;
		}
		executing_coroutines = false;

		for (auto g : peeding_add_coroutines)
			start_coroutine(g);
	}

	struct cBpInstanceCreate : cBpInstance::Create
	{
		cBpInstancePtr operator()(EntityPtr e) override
		{
			return new cBpInstancePrivate();
		}
	}cBpInstance_create;
	cBpInstance::Create& cBpInstance::create = cBpInstance_create;
}
