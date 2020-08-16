#pragma once

#include <flame/universe/components/combobox.h>

namespace flame
{
	struct cTextPrivate;
	struct cMenuPrivate;

	struct cComboboxPrivate : cCombobox // R ~ on_*
	{
		int index = -1;

		cTextPrivate* text = nullptr; // R ref
		cMenuPrivate* menu = nullptr; // R ref

		int get_index() const override { return index; }
		void set_index(int index) override;
	};

	struct cComboboxItemPrivate : cComboboxItem // R ~ on_*
	{
		cComboboxPrivate* combobox = nullptr; // R ref place=ancestor
		cComboboxPrivate* staging_combobox = nullptr;

		cEventReceiverPrivate* event_receiver = nullptr; // R ref

		void* mouse_listener = nullptr;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
		void on_lost_combobox();
	};
}
