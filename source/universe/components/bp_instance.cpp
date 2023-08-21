#include "../../foundation/blueprint.h"
#include "../entity_private.h"
#include "bp_instance_private.h"
#include "../blueprint_library/library.h"

namespace flame
{
	cBpInstancePrivate::~cBpInstancePrivate()
	{
		if (bp_ins)
			delete bp_ins;
		if (bp)
			Blueprint::release(bp);
	}

	void cBpInstancePrivate::set_bp_name(const std::filesystem::path& name)
	{
		if (bp_name == name)
			return;

		if (bp_ins)
		{
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
			bp_ins = BlueprintInstance::create(bp);
	}

	void cBpInstancePrivate::start()
	{
		if (bp_ins)
		{
			if (bp_ins->prepare_executing("start"_h))
			{
				bp_self = entity;
				bp_ins->run();
				bp_self = nullptr;
			}
		}
	}

	void cBpInstancePrivate::update()
	{
		if (bp_ins)
		{
			if (bp_ins->prepare_executing("update"_h))
			{
				bp_self = entity;
				bp_ins->run();
				bp_self = nullptr;
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
