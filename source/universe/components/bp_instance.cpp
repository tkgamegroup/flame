#include "../entity_private.h"
#include "bp_instance_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cBpInstancePrivate::~cBpInstancePrivate()
	{
		if (bp_ins)
		{
			// dont use static blueprints in two or more objects
			for (auto& kv : bp_ins->groups)
			{
				if (kv.second.trigger_message)
					bp_ins->unregister_group(&kv.second);
			}

			if (on_gui_cb)
				sRenderer::instance()->hud_callbacks.remove((uint)this);

			if (!bp_ins->is_static)
				delete bp_ins;
		}
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
			// dont use static blueprints in two or more objects
			for (auto& kv : bp_ins->groups)
			{
				if (kv.second.trigger_message)
					bp_ins->unregister_group(&kv.second);
			}

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
			bp_ins->set_variable_as("self"_h, entity);
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
		assert(group->execution_type == BlueprintExecutionCoroutine);

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
			// dont use static blueprints in two or more objects
			for (auto& kv : bp_ins->groups)
			{
				if (kv.second.trigger_message)
					bp_ins->register_group(&kv.second);
			}

			if (auto g = bp_ins->find_group("start"_h); g)
			{
				if (g->execution_type == BlueprintExecutionCoroutine)
					start_coroutine(g);
				else
				{
					bp_ins->prepare_executing(g);
					bp_ins->run(g);
				}
			}
			if (auto g = bp_ins->find_group("update"_h); g)
			{
				assert(g->execution_type == BlueprintExecutionFunction);
				update_cb = g;
			}
			if (auto g = bp_ins->find_group("on_gui"_h); g)
			{
				assert(g->execution_type == BlueprintExecutionFunction);
				on_gui_cb = g;
				sRenderer::instance()->hud_callbacks.add([this]() {
					bp_ins->prepare_executing(on_gui_cb);
					bp_ins->run(on_gui_cb);
				}, (uint)this);
			}
		}
	}

	void cBpInstancePrivate::update()
	{
		if (update_cb)
		{
			bp_ins->prepare_executing(update_cb);
			bp_ins->run(update_cb);
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
