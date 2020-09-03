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
		do_file();
		Entity::report_data_changed(this, S<ch("src")>::v);
	}

	void cScriptPrivate::do_file()
	{
		if (!src.empty() && entity)
		{
			auto ins = script::Instance::get();
			ins->add_object(entity, "entity", "Entity");
			ins->excute(src.c_str());
		}
	}

	void cScriptPrivate::on_added()
	{
		do_file();
	}

	cScript* cScript::create()
	{
		return f_new<cScriptPrivate>();
	}
}
