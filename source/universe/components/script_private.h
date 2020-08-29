#pragma once

#include <flame/universe/components/script.h>

namespace flame
{
	struct cScriptPrivate : cScript // R ~ on_*
	{
		std::filesystem::path filename;

		const wchar_t* get_filename() const override { return filename.c_str(); }
		void set_filename(const wchar_t* fn) override;

		void do_file();

		void on_added() override;
	};
}
