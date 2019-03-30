namespace flame
{
	struct UI
	{
		int key_states[Key_count];

		Ivec2 mouse_pos;
		Ivec2 mouse_disp;
		int mouse_scroll;

		int mouse_buttons[MouseKey_count];

		bool processed_mouse_input;
		bool processed_keyboard_input;

		FLAME_UNIVERSE_EXPORTS graphics::Canvas* canvas();
		FLAME_UNIVERSE_EXPORTS Ivec2 size() const;

		FLAME_UNIVERSE_EXPORTS void on_key(KeyState action, int value);
		FLAME_UNIVERSE_EXPORTS void on_mouse(KeyState action, MouseKey key, const Ivec2 & pos);
		FLAME_UNIVERSE_EXPORTS void on_resize(const Ivec2 & size);

		FLAME_UNIVERSE_EXPORTS Element* root();
		FLAME_UNIVERSE_EXPORTS Element* hovering_element();
		FLAME_UNIVERSE_EXPORTS Element* focus_element();
		FLAME_UNIVERSE_EXPORTS Element* key_focus_element();
		FLAME_UNIVERSE_EXPORTS Element* dragging_element();
		FLAME_UNIVERSE_EXPORTS Element* popup_element();
		FLAME_UNIVERSE_EXPORTS void set_hovering_element(Element * w);
		FLAME_UNIVERSE_EXPORTS void set_focus_element(Element * w);
		FLAME_UNIVERSE_EXPORTS void set_key_focus_element(Element * w);
		FLAME_UNIVERSE_EXPORTS void set_dragging_element(Element * w);
		FLAME_UNIVERSE_EXPORTS void set_popup_element(Element * w, bool modual = false);
		FLAME_UNIVERSE_EXPORTS void close_popup();

		FLAME_UNIVERSE_EXPORTS void step(float elp_time, const Vec2& show_off = Vec2(0.f));

		FLAME_UNIVERSE_EXPORTS float total_time() const;
	};

	typedef UI* UIPtr;
}
