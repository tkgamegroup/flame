#include "key.h"

Key key_map[KEY_COUNT];
const wchar_t* key_names[KEY_COUNT];

void init_key()
{
	key_map[KEY_PAUSE] = Keyboard_Esc;
	key_map[KEY_LEFT] = Keyboard_A;
	key_map[KEY_RIGHT] = Keyboard_D;
	key_map[KEY_ROTATE_LEFT] = Keyboard_J;
	key_map[KEY_ROTATE_RIGHT] = Keyboard_K;
	key_map[KEY_SOFT_DROP] = Keyboard_S;
	key_map[KEY_HARD_DROP] = Keyboard_Space;
	key_map[KEY_HOLD] = Keyboard_E;

	key_names[KEY_PAUSE] = L"Pause";
	key_names[KEY_LEFT] = L"Left";
	key_names[KEY_RIGHT] = L"Right";
	key_names[KEY_ROTATE_LEFT] = L"Rotate_Left";
	key_names[KEY_ROTATE_RIGHT] = L"Rotate_Right";
	key_names[KEY_SOFT_DROP] = L"Soft_Drop";
	key_names[KEY_HARD_DROP] = L"Hard_Drop";
	key_names[KEY_HOLD] = L"Hold";
}
