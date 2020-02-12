#pragma once

#include <flame/foundation/foundation.h>

using namespace flame;

enum
{
	KEY_PAUSE,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_ROTATE_LEFT,
	KEY_ROTATE_RIGHT,
	KEY_SOFT_DROP,
	KEY_HARD_DROP,
	KEY_HOLD,
	KEY_COUNT
};

extern Key key_map[KEY_COUNT];

void init_key();

