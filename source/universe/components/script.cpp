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
			entity->component_data_changed(this, S<"src"_h>);
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
		if (!content.empty())
		{
			if (!scr_ins->excute(content.c_str()))
				fassert(0);
		}
		if (!src.empty())
		{
			for (auto& s : SUW::split_with_spaces(src.wstring()))
			{
				if (!scr_ins->excute_file(s.c_str()))
					fassert(0);
			}
		}
	}

	cScript* cScript::create(void* parms)
	{
		return new cScriptPrivate();
	}
}
