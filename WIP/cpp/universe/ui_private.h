#include <flame/foundation/window.h>
#include <flame/universe/element.h>
#include <flame/universe/ui.h>

namespace flame
{
	struct UIPrivate : UI
	{
		Element* key_focus_element_;
		Element* popup_element_;
		bool popup_element_modual_;
		Ivec2 mouse_prev_pos_;
		Element* potential_doubleclick_element_;
		float doubleclick_timer_;
		std::vector<int> keydown_inputs_;
		std::vector<int> keyup_inputs_;
		std::vector<wchar_t> char_inputs_;
		bool char_input_compelete_;
	};
}

