namespace flame
{
	struct UI
	{
		int key_states[Key_count];

		bool processed_mouse_input;
		bool processed_keyboard_input;

		FLAME_UNIVERSE_EXPORTS Element* popup_element();
		FLAME_UNIVERSE_EXPORTS void set_popup_element(Element * w, bool modual = false);
		FLAME_UNIVERSE_EXPORTS void close_popup();
	};

	typedef UI* UIPtr;
}
