#pragma once

#include "input_dialog.h"

namespace flame
{
	struct dInputDialogPrivate  : dInputDialog
	{
		cTextPrivate* text;

		std::vector<std::unique_ptr<Closure<void(Capture&, bool, const wchar_t*)>>> callbacks;

		void set_text(const wchar_t* text) override;

		void* add_callback(void (*callback)(Capture& c, bool ok, const wchar_t* text), const Capture& capture) override;
		void remove_callback(void* ret) override;

		void on_load_finished() override;
	};
}
