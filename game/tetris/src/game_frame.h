#pragma once

#include <flame/math.h>
#include <flame/UI/widget.h>

struct TetrisType;

struct Tetris
{
	enum { hori_size = 4 };
	enum { vert_size = 4 };

	flame::Ivec2 pos;
	TetrisType *t;
	int transform_id;
	int x_move, y_move;
	bool transform;
	bool down;
	int gear;

	void reset();
	void spawn();
	void take_away();
	bool try_pos(const flame::Ivec2 &off);
	void print();
};

struct TetrisType
{
	bool v[4][Tetris::hori_size * Tetris::vert_size];
	float h;
};

extern TetrisType tetris_types[];

void create_game_frame();
void destroy_game_frame();
