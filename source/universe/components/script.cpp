#include <flame/script/script.h>
#include "../entity_private.h"
#include "script_private.h"

namespace flame
{
	void cScriptPrivate::set_src(const wchar_t* fn)
	{
		if (src == fn)
			return;
		src = fn;
		if (entity)
			entity->data_changed(this, S<"src"_h>);
	}

	void cScriptPrivate::on_entered_world()
	{
		if (!src.empty())
		{
			auto ins = script::Instance::get_default();
			ins->add_object(entity, "entity", "Entity");
			ins->excute(src.c_str());
		}
	}

	cScript* cScript::create()
	{
		return f_new<cScriptPrivate>();
	}
}
