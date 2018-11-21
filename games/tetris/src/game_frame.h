// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
