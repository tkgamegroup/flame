#pragma once

#include <flame/math.h>

using namespace flame;

enum MinoType
{
	Mino_L,
	Mino_T,
	Mino_S,
	Mino_Z,
	Mino_J,
	Mino_O,
	Mino_I,

	MinoTypeCount
};

extern Vec2i g_mino_coords[7][3];

extern Vec2i g_mino_LTSZJ_offsets[5][4];
extern Vec2i g_mino_O_offsets[5][4];
extern Vec2i g_mino_I_offsets[5][4];

void init_mino();
