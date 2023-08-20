#include "../../foundation/blueprint.h"
#include "../entity_private.h"
#include "bp_instance_private.h"

namespace flame
{
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

	struct cBpInstanceCreate : cBpInstance::Create
	{
		cBpInstancePtr operator()(EntityPtr e) override
		{
			return new cBpInstancePrivate();
		}
	}cBpInstance_create;
	cBpInstance::Create& cBpInstance::create = cBpInstance_create;
}
