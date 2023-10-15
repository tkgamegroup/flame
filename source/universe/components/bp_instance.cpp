#include "../../foundation/blueprint.h"
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
			if (!bp->name.empty())
				bp_ins = BlueprintInstance::get(bp->name_hash);
			else
				bp_ins = BlueprintInstance::create(bp);
			bp_ins->set_variable("self"_h, entity);
		}
	}

	void cBpInstancePrivate::start()
	{
		if (bp_ins)
		{
			if (auto g = bp_ins->get_group("start"_h))
			{
				bp_ins->prepare_executing(g);
				bp_ins->run(g);
			}
		}
	}

	void cBpInstancePrivate::update()
	{
		if (bp_ins)
		{
			if (auto g = bp_ins->get_group("update"_h); g)
			{
				bp_ins->prepare_executing(g);
				bp_ins->run(g);
			}
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
