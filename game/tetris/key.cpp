#include "key.h"

Key key_map[KEY_COUNT];

void init_key()
{
	key_map[KEY_LEFT] = Key_J;
	key_map[KEY_RIGHT] = Key_K;
	key_map[KEY_ROTATE_LEFT] = Key_U;
	key_map[KEY_ROTATE_RIGHT] = Key_I;
	key_map[KEY_SOFT_DROP] = Key_F;
	key_map[KEY_HARD_DROP] = Key_Space;
}
