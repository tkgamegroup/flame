#pragma once

#include <flame/universe/drivers/input_dialog.h>

namespace flame
{
	struct cTextPrivate;

	struct dInputDialogPrivate  : dInputDialog
	{
		cTextPrivate* text;

		std::vector<std::unique_ptr<Closure<void(Capture&, bool, const wchar_t*)>>> callbacks;

		void* add_callback(void (*callback)(Capture& c, bool ok, const wchar_t* text), const Capture& capture) override;
		void remove_callback(void* ret) override;

		void on_load_finished() override;
	};
}
