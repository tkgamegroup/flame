#pragma once

struct MinoData
{
	bool tiles[4][4][4];
	Vec2u offsets[4][4];

	enum { start_x = 3 };
	enum { start_y = 2 };

	MinoData()
	{
		for (auto i = 0; i < 4; i++)
		{
			for (auto x = 0; x < 4; x++)
			{
				for (auto y = 0; y < 4; y++)
					tiles[i][x][y] = 0;
			}
		}
		for (auto i = 0; i < 4; i++)
		{
			for (auto j = 0; j < 4; j++)
				offsets[i][j] = Vec2u(0);
		}
	}

}mino_datas[7];

inline void init_minos()
{
	{
		auto& m = mino_datas[0]; // L

		m.tiles[0][2][0] = true;
		m.tiles[0][0][1] = true;
		m.tiles[0][1][1] = true;
		m.tiles[0][2][1] = true;

		m.offsets[0][0] = Vec2u(0, 0);
		m.offsets[0][1] = Vec2u(0, 0);
		m.offsets[0][2] = Vec2u(0, 0);
		m.offsets[0][3] = Vec2u(0, 0);

		m.tiles[1][1][0] = true;
		m.tiles[1][1][1] = true;
		m.tiles[1][1][2] = true;
		m.tiles[1][2][2] = true;

		m.offsets[1][0] = Vec2u(-1, 0);
		m.offsets[1][1] = Vec2u(-1, -1);
		m.offsets[1][2] = Vec2u(0, 2);
		m.offsets[1][3] = Vec2u(-1, 2);

		m.tiles[2][0][1] = true;
		m.tiles[2][1][1] = true;
		m.tiles[2][2][1] = true;
		m.tiles[2][0][2] = true;

		m.offsets[2][0] = Vec2u(0, 0);
		m.offsets[2][1] = Vec2u(0, 0);
		m.offsets[2][2] = Vec2u(0, 0);
		m.offsets[2][3] = Vec2u(0, 0);

		m.tiles[3][0][0] = true;
		m.tiles[3][1][0] = true;
		m.tiles[3][1][1] = true;
		m.tiles[3][1][2] = true;

		m.offsets[3][0] = Vec2u(1, 0);
		m.offsets[3][1] = Vec2u(1, -1);
		m.offsets[3][2] = Vec2u(0, 2);
		m.offsets[3][3] = Vec2u(1, 2);
	}
	{
		auto& m = mino_datas[1]; // T

		m.tiles[0][1][0] = true;
		m.tiles[0][0][1] = true;
		m.tiles[0][1][1] = true;
		m.tiles[0][2][1] = true;

		m.offsets[0][0] = Vec2u(0, 0);
		m.offsets[0][1] = Vec2u(0, 0);
		m.offsets[0][2] = Vec2u(0, 0);
		m.offsets[0][3] = Vec2u(0, 0);

		m.tiles[1][1][0] = true;
		m.tiles[1][1][1] = true;
		m.tiles[1][1][2] = true;
		m.tiles[1][2][1] = true;

		m.offsets[1][0] = Vec2u(-1, 0);
		m.offsets[1][1] = Vec2u(-1, -1);
		m.offsets[1][2] = Vec2u(0, 2);
		m.offsets[1][3] = Vec2u(-1, 2);

		m.tiles[2][0][1] = true;
		m.tiles[2][1][1] = true;
		m.tiles[2][2][1] = true;
		m.tiles[2][1][2] = true;

		m.offsets[2][0] = Vec2u(0, 0);
		m.offsets[2][1] = Vec2u(0, 0);
		m.offsets[2][2] = Vec2u(0, 0);
		m.offsets[2][3] = Vec2u(0, 0);

		m.tiles[3][1][0] = true;
		m.tiles[3][1][1] = true;
		m.tiles[3][1][2] = true;
		m.tiles[3][0][1] = true;

		m.offsets[3][0] = Vec2u(1, 0);
		m.offsets[3][1] = Vec2u(1, -1);
		m.offsets[3][2] = Vec2u(0, 2);
		m.offsets[3][3] = Vec2u(1, 2);
	}
	{
		auto& m = mino_datas[2]; // S

		m.tiles[0][1][0] = true;
		m.tiles[0][2][0] = true;
		m.tiles[0][0][1] = true;
		m.tiles[0][1][1] = true;

		m.offsets[0][0] = Vec2u(0, 0);
		m.offsets[0][1] = Vec2u(0, 0);
		m.offsets[0][2] = Vec2u(0, 0);
		m.offsets[0][3] = Vec2u(0, 0);

		m.tiles[1][1][0] = true;
		m.tiles[1][1][1] = true;
		m.tiles[1][2][1] = true;
		m.tiles[1][2][2] = true;

		m.offsets[1][0] = Vec2u(-1, 0);
		m.offsets[1][1] = Vec2u(-1, -1);
		m.offsets[1][2] = Vec2u(0, 2);
		m.offsets[1][3] = Vec2u(-1, 2);

		m.tiles[2][1][1] = true;
		m.tiles[2][2][1] = true;
		m.tiles[2][0][2] = true;
		m.tiles[2][1][2] = true;

		m.offsets[2][0] = Vec2u(0, 0);
		m.offsets[2][1] = Vec2u(0, 0);
		m.offsets[2][2] = Vec2u(0, 0);
		m.offsets[2][3] = Vec2u(0, 0);

		m.tiles[3][0][0] = true;
		m.tiles[3][0][1] = true;
		m.tiles[3][1][1] = true;
		m.tiles[3][1][2] = true;

		m.offsets[3][0] = Vec2u(1, 0);
		m.offsets[3][1] = Vec2u(1, -1);
		m.offsets[3][2] = Vec2u(0, 2);
		m.offsets[3][3] = Vec2u(1, 2);
	}
	{
		auto& m = mino_datas[3]; // Z

		m.tiles[0][0][0] = true;
		m.tiles[0][1][0] = true;
		m.tiles[0][1][1] = true;
		m.tiles[0][2][1] = true;

		m.offsets[0][0] = Vec2u(0, 0);
		m.offsets[0][1] = Vec2u(0, 0);
		m.offsets[0][2] = Vec2u(0, 0);
		m.offsets[0][3] = Vec2u(0, 0);

		m.tiles[1][2][0] = true;
		m.tiles[1][2][1] = true;
		m.tiles[1][1][1] = true;
		m.tiles[1][1][2] = true;

		m.offsets[1][0] = Vec2u(-1, 0);
		m.offsets[1][1] = Vec2u(-1, -1);
		m.offsets[1][2] = Vec2u(0, 2);
		m.offsets[1][3] = Vec2u(-1, 2);

		m.tiles[2][0][1] = true;
		m.tiles[2][1][1] = true;
		m.tiles[2][1][2] = true;
		m.tiles[2][2][2] = true;

		m.offsets[2][0] = Vec2u(0, 0);
		m.offsets[2][1] = Vec2u(0, 0);
		m.offsets[2][2] = Vec2u(0, 0);
		m.offsets[2][3] = Vec2u(0, 0);

		m.tiles[3][1][0] = true;
		m.tiles[3][1][1] = true;
		m.tiles[3][0][1] = true;
		m.tiles[3][0][2] = true;

		m.offsets[3][0] = Vec2u(1, 0);
		m.offsets[3][1] = Vec2u(1, -1);
		m.offsets[3][2] = Vec2u(0, 2);
		m.offsets[3][3] = Vec2u(1, 2);
	}
	{
		auto& m = mino_datas[4]; // J

		m.tiles[0][0][0] = true;
		m.tiles[0][0][1] = true;
		m.tiles[0][1][1] = true;
		m.tiles[0][2][1] = true;

		m.offsets[0][0] = Vec2u(0, 0);
		m.offsets[0][1] = Vec2u(0, 0);
		m.offsets[0][2] = Vec2u(0, 0);
		m.offsets[0][3] = Vec2u(0, 0);

		m.tiles[1][1][0] = true;
		m.tiles[1][1][1] = true;
		m.tiles[1][1][2] = true;
		m.tiles[1][2][0] = true;

		m.offsets[1][0] = Vec2u(-1, 0);
		m.offsets[1][1] = Vec2u(-1, -1);
		m.offsets[1][2] = Vec2u(0, 2);
		m.offsets[1][3] = Vec2u(-1, 2);

		m.tiles[2][0][1] = true;
		m.tiles[2][1][1] = true;
		m.tiles[2][2][1] = true;
		m.tiles[2][2][2] = true;

		m.offsets[2][0] = Vec2u(0, 0);
		m.offsets[2][1] = Vec2u(0, 0);
		m.offsets[2][2] = Vec2u(0, 0);
		m.offsets[2][3] = Vec2u(0, 0);

		m.tiles[3][1][0] = true;
		m.tiles[3][1][1] = true;
		m.tiles[3][1][2] = true;
		m.tiles[3][0][2] = true;

		m.offsets[3][0] = Vec2u(1, 0);
		m.offsets[3][1] = Vec2u(1, -1);
		m.offsets[3][2] = Vec2u(0, 2);
		m.offsets[3][3] = Vec2u(1, 2);
	}
	{
		auto& m = mino_datas[5]; // O

		m.tiles[0][1][0] = true;
		m.tiles[0][2][0] = true;
		m.tiles[0][1][1] = true;
		m.tiles[0][2][1] = true;

		m.offsets[0][0] = Vec2u(0, 0);
		m.offsets[0][1] = Vec2u(0, 0);
		m.offsets[0][2] = Vec2u(0, 0);
		m.offsets[0][3] = Vec2u(0, 0);

		m.tiles[1][1][0] = true;
		m.tiles[1][2][0] = true;
		m.tiles[1][1][1] = true;
		m.tiles[1][2][1] = true;

		m.offsets[1][0] = Vec2u(0, 0);
		m.offsets[1][1] = Vec2u(0, 0);
		m.offsets[1][2] = Vec2u(0, 0);
		m.offsets[1][3] = Vec2u(0, 0);

		m.tiles[2][1][0] = true;
		m.tiles[2][2][0] = true;
		m.tiles[2][1][1] = true;
		m.tiles[2][2][1] = true;

		m.offsets[2][0] = Vec2u(0, 0);
		m.offsets[2][1] = Vec2u(0, 0);
		m.offsets[2][2] = Vec2u(0, 0);
		m.offsets[2][3] = Vec2u(0, 0);

		m.tiles[3][1][0] = true;
		m.tiles[3][2][0] = true;
		m.tiles[3][1][1] = true;
		m.tiles[3][2][1] = true;

		m.offsets[3][0] = Vec2u(0, 0);
		m.offsets[3][1] = Vec2u(0, 0);
		m.offsets[3][2] = Vec2u(0, 0);
		m.offsets[3][3] = Vec2u(0, 0);
	}
	{
		auto& m = mino_datas[6]; // I

		m.tiles[0][0][1] = true;
		m.tiles[0][1][1] = true;
		m.tiles[0][2][1] = true;
		m.tiles[0][3][1] = true;

		m.offsets[0][0] = Vec2u(1, 0);
		m.offsets[0][1] = Vec2u(-2, 0);
		m.offsets[0][2] = Vec2u(1, 0);
		m.offsets[0][3] = Vec2u(-2, 0);

		m.tiles[1][2][0] = true;
		m.tiles[1][2][1] = true;
		m.tiles[1][2][2] = true;
		m.tiles[1][2][3] = true;

		m.offsets[1][0] = Vec2u(-1, 0);
		m.offsets[1][1] = Vec2u(-1, 0);
		m.offsets[1][2] = Vec2u(-1, 1);
		m.offsets[1][3] = Vec2u(-1, -2);

		m.tiles[2][0][2] = true;
		m.tiles[2][1][2] = true;
		m.tiles[2][2][2] = true;
		m.tiles[2][3][2] = true;

		m.offsets[2][0] = Vec2u(-2, 0);
		m.offsets[2][1] = Vec2u(1, 0);
		m.offsets[2][2] = Vec2u(-2, -1);
		m.offsets[2][3] = Vec2u(1, -1);

		m.tiles[3][1][0] = true;
		m.tiles[3][1][1] = true;
		m.tiles[3][1][2] = true;
		m.tiles[3][1][3] = true;

		m.offsets[3][0] = Vec2u(0, 0);
		m.offsets[3][1] = Vec2u(0, 0);
		m.offsets[3][2] = Vec2u(0, -2);
		m.offsets[3][3] = Vec2u(0, 1);
	}
}
