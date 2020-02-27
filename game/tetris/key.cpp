#include "key.h"

Key key_map[KEY_COUNT];
const wchar_t* key_names[KEY_COUNT];

void init_key()
{
	key_map[KEY_PAUSE] = Key_Esc;
	key_map[KEY_LEFT] = Key_A;
	key_map[KEY_RIGHT] = Key_D;
	key_map[KEY_ROTATE_LEFT] = Key_J;
	key_map[KEY_ROTATE_RIGHT] = Key_K;
	key_map[KEY_SOFT_DROP] = Key_S;
	key_map[KEY_HARD_DROP] = Key_Space;
	key_map[KEY_HOLD] = Key_E;

	key_names[KEY_PAUSE] = L"Pause";
	key_names[KEY_LEFT] = L"Left";
	key_names[KEY_RIGHT] = L"Right";
	key_names[KEY_ROTATE_LEFT] = L"Rotate_Left";
	key_names[KEY_ROTATE_RIGHT] = L"Rotate_Right";
	key_names[KEY_SOFT_DROP] = L"Soft_Drop";
	key_names[KEY_HARD_DROP] = L"Hard_Drop";
	key_names[KEY_HOLD] = L"Hold";
}
