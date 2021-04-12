#pragma once

#include "script.h"

namespace flame
{
	struct cScriptPrivate : cScript
	{
		std::filesystem::path src;

		std::string content;

		bool first = true;

		const wchar_t* get_src() const override { return src.c_str(); }
		void set_src(const wchar_t* fn) override;

		const char* get_content() const override { return content.c_str(); };
		void set_content(const char* content) override;

		void on_entered_world() override;
	};
}
