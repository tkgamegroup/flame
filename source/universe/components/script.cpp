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

	void cScriptPrivate::set_content(const char* _content)
	{
		content = _content;
	}

	void cScriptPrivate::on_entered_world()
	{
		if (!first)
			return;
		first = false;

		auto scr_ins = script::Instance::get_default();
		scr_ins->push_object();
		scr_ins->push_pointer(entity);
		scr_ins->set_member_name("p");
		scr_ins->set_object_type("flame::Entity");
		scr_ins->set_global_name("entity");
		if (!src.empty())
			scr_ins->excute_file(src.c_str());
		if (!content.empty())
			scr_ins->excute(content.c_str());
	}

	cScript* cScript::create()
	{
		return f_new<cScriptPrivate>();
	}
}
