#pragma once

#include <flame/universe/components/script.h>

namespace flame
{
	struct cScriptPrivate : cScript
	{
		std::filesystem::path src;

		const wchar_t* get_src() const override { return src.c_str(); }
		void set_src(const wchar_t* fn) override;

		void on_entered_world() override;
	};
}
