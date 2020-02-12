#include "key.h"

Key key_map[KEY_COUNT];

void init_key()
{
	key_map[KEY_PAUSE] = Key_Esc;
	key_map[KEY_LEFT] = Key_Left;
	key_map[KEY_RIGHT] = Key_Right;
	key_map[KEY_ROTATE_LEFT] = Key_Z;
	key_map[KEY_ROTATE_RIGHT] = Key_Up;
	key_map[KEY_SOFT_DROP] = Key_Down;
	key_map[KEY_HARD_DROP] = Key_Space;
	key_map[KEY_HOLD] = Key_C;
}
