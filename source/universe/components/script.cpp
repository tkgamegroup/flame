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
			looper().add_event([](Capture& c) {
				auto thiz = c.thiz<cScriptPrivate>();
				auto ins = script::Instance::get();
				ins->add_object(thiz->entity, "entity", "Entity");
				ins->excute(thiz->src.c_str());
			}, Capture().set_thiz(this));
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
