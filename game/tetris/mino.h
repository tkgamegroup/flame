#pragma once

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

Vec2i g_mino_coords[7][3];

Vec2i g_mino_LTSZJ_offsets[5][4];
Vec2i g_mino_O_offsets[5][4];
Vec2i g_mino_I_offsets[5][4];

inline void init()
{
	g_mino_coords[Mino_L][0] = Vec2i(-1, +0);
	g_mino_coords[Mino_L][1] = Vec2i(+1, +0);
	g_mino_coords[Mino_L][2] = Vec2i(+1, -1);

	g_mino_coords[Mino_T][0] = Vec2i(-1, +0);
	g_mino_coords[Mino_T][1] = Vec2i(+1, +0);
	g_mino_coords[Mino_T][2] = Vec2i(+0, -1);

	g_mino_coords[Mino_S][0] = Vec2i(-1, +0);
	g_mino_coords[Mino_S][1] = Vec2i(+0, -1);
	g_mino_coords[Mino_S][2] = Vec2i(+1, -1);

	g_mino_coords[Mino_Z][0] = Vec2i(-1, -1);
	g_mino_coords[Mino_Z][1] = Vec2i(+0, -1);
	g_mino_coords[Mino_Z][2] = Vec2i(+1, +0);

	g_mino_coords[Mino_J][0] = Vec2i(-1, +0);
	g_mino_coords[Mino_J][1] = Vec2i(+1, +0);
	g_mino_coords[Mino_J][2] = Vec2i(-1, -1);

	g_mino_coords[Mino_O][0] = Vec2i(+0, -1);
	g_mino_coords[Mino_O][1] = Vec2i(+1, +0);
	g_mino_coords[Mino_O][2] = Vec2i(+1, -1);

	g_mino_coords[Mino_I][0] = Vec2i(-1, +0);
	g_mino_coords[Mino_I][1] = Vec2i(+1, +0);
	g_mino_coords[Mino_I][2] = Vec2i(+2, +0);

	memset(g_mino_LTSZJ_offsets, 0, sizeof(g_mino_LTSZJ_offsets));
	memset(g_mino_O_offsets, 0, sizeof(g_mino_O_offsets));
	memset(g_mino_I_offsets, 0, sizeof(g_mino_I_offsets));

	g_mino_LTSZJ_offsets[0][0] = Vec2i(+0, +0);
	g_mino_LTSZJ_offsets[0][1] = Vec2i(+0, +0);
	g_mino_LTSZJ_offsets[0][2] = Vec2i(+0, +0);
	g_mino_LTSZJ_offsets[0][3] = Vec2i(+0, +0);

	g_mino_LTSZJ_offsets[1][0] = Vec2i(+0, +0);
	g_mino_LTSZJ_offsets[1][1] = Vec2i(+1, +0);
	g_mino_LTSZJ_offsets[1][2] = Vec2i(+0, +0);
	g_mino_LTSZJ_offsets[1][3] = Vec2i(-1, +0);

	g_mino_LTSZJ_offsets[2][0] = Vec2i(+0, +0);
	g_mino_LTSZJ_offsets[2][1] = Vec2i(+1, +1);
	g_mino_LTSZJ_offsets[2][2] = Vec2i(+0, +0);
	g_mino_LTSZJ_offsets[2][3] = Vec2i(-1, +1);

	g_mino_LTSZJ_offsets[3][0] = Vec2i(+0, +0);
	g_mino_LTSZJ_offsets[3][1] = Vec2i(+0, -2);
	g_mino_LTSZJ_offsets[3][2] = Vec2i(+0, +0);
	g_mino_LTSZJ_offsets[3][3] = Vec2i(+0, -2);

	g_mino_LTSZJ_offsets[4][0] = Vec2i(+0, +0);
	g_mino_LTSZJ_offsets[4][1] = Vec2i(+1, -2);
	g_mino_LTSZJ_offsets[4][2] = Vec2i(+0, +0);
	g_mino_LTSZJ_offsets[4][3] = Vec2i(-1, -2);

	g_mino_I_offsets[0][0] = Vec2i(+0, +0);
	g_mino_I_offsets[0][1] = Vec2i(-1, +0);
	g_mino_I_offsets[0][2] = Vec2i(-1, -1);
	g_mino_I_offsets[0][3] = Vec2i(+0, -1);

	g_mino_I_offsets[1][0] = Vec2i(-1, +0);
	g_mino_I_offsets[1][1] = Vec2i(+0, +0);
	g_mino_I_offsets[1][2] = Vec2i(+1, -1);
	g_mino_I_offsets[1][3] = Vec2i(+0, -1);

	g_mino_I_offsets[2][0] = Vec2i(+2, +0);
	g_mino_I_offsets[2][1] = Vec2i(+0, +0);
	g_mino_I_offsets[2][2] = Vec2i(-2, -1);
	g_mino_I_offsets[2][3] = Vec2i(+0, -1);

	g_mino_I_offsets[3][0] = Vec2i(-1, +0);
	g_mino_I_offsets[3][1] = Vec2i(+0, -1);
	g_mino_I_offsets[3][2] = Vec2i(+1, +0);
	g_mino_I_offsets[3][3] = Vec2i(+0, +1);

	g_mino_I_offsets[4][0] = Vec2i(+2, +0);
	g_mino_I_offsets[4][1] = Vec2i(+0, +2);
	g_mino_I_offsets[4][2] = Vec2i(-2, +0);
	g_mino_I_offsets[4][3] = Vec2i(+0, -2);

	g_mino_O_offsets[0][0] = Vec2i(+0, +0);
	g_mino_O_offsets[0][1] = Vec2i(+0, +1);
	g_mino_O_offsets[0][2] = Vec2i(-1, +1);
	g_mino_O_offsets[0][3] = Vec2i(-1, +0);
}
