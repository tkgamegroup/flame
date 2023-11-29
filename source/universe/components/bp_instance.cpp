
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
			if (g->wait_time > 0.f)
			{
				g->wait_time -= delta_time;
				if (g->wait_time <= 0.f)
					g->wait_time = 0.f;
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

		bp_ins->prepare_executing(group);
		group->wait_time = delay;
		if (run_coroutine(group))
			co_routines.push_back(group);
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
		for (auto it = co_routines.begin(); it != co_routines.end();)
		{
			auto g = *it;
			if (!run_coroutine(g))
				it = co_routines.erase(it);
			else
				it++;
		}
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
