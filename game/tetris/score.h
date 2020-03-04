#pragma once

#include <flame/serialize.h>

inline void get_combo_award(uint combo, uint& attack, std::wstring& str)
{
	if (combo >= 1)
	{
		if (combo > 12)
			attack += 5;
		else
		{
			switch (combo)
			{
			case 2:
				attack += 1;
				break;
			case 3:
				attack += 1;
				break;
			case 4:
				attack += 2;
				break;
			case 5:
				attack += 2;
				break;
			case 6:
				attack += 3;
				break;
			case 7:
				attack += 3;
				break;
			case 8:
				attack += 4;
				break;
			case 9:
				attack += 4;
				break;
			case 10:
				attack += 4;
				break;
			case 11:
				attack += 5;
				break;
			case 12:
				attack += 5;
				break;
			}
		}
		str += wfmt(L"Ren %d\n", combo);
	}
}

inline void get_lines_award(uint l, bool tspin, bool mini, bool& back_to_back, uint& score, uint& attack, std::wstring& str)
{
	switch (l)
	{
	case 1:
		if (tspin)
		{
			if (mini)
			{
				score += 400;
				str += L"T-Spin\nMini\n";
			}
			else
			{
				score += 800;
				attack += 2;
				str += L"T-Spin\nSingle\n";
			}
			if (back_to_back)
			{
				score += mini ? 200 : 400;
				attack++;
				str += L"\nBack\nTo\nBack";
			}
			back_to_back = true;
		}
		else
			back_to_back = false;
		break;
	case 2:
		if (tspin)
		{
			score += 1200;
			attack += 4;
			str += L"T-Spin\nDouble\n";
			if (back_to_back)
			{
				score += 600;
				attack++;
				str += L"\nBack\nTo\nBack";
			}
			back_to_back = true;
		}
		else
		{
			score += 300;
			attack += 1;
			str += L"Double\n";
			back_to_back = false;
		}
		break;
	case 3:
		if (tspin)
		{
			score += 1600;
			attack += 6;
			str += L"T-Spin\nTriple\n";
			if (back_to_back)
			{
				score += 800;
				attack++;
				str += L"\nBack\nTo\nBack";
			}
			back_to_back = true;
		}
		else
		{
			score += 500;
			attack += 2;
			str += L"Triple\n";
			back_to_back = false;
		}
		break;
	case 4:
		score += 800;
		attack += 4;

		str += L"Tetris\n";
		if (back_to_back)
		{
			score += 400;
			attack++;
			str += L"\nBack\nTo\nBack";
		}
		back_to_back = true;
		break;
	}
}
