#include <flame/foundation/foundation.h>
#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
#include <flame/graphics/noise.h>

using namespace flame;

struct P
{
	float x;
	float y;
};

template <>
struct std::formatter<P, char> : std::formatter<std::string>
{
	auto format(P p, std::format_context& ctx) const
	{
		auto str = std::format("{}, {}", p.x, p.y);
		return std::copy(str.begin(), str.end(), ctx.out());
	}
};

template <>
struct std::formatter<P, wchar_t> : std::formatter<std::wstring>
{
	auto format(P p, std::wformat_context& ctx) const
	{
		auto str = std::format(L"{}, {}", p.x, p.y);
		return std::copy(str.begin(), str.end(), ctx.out());
	}
};

int main(int argc, char** args) 
{
	P p;
	std::format(L"", p);
	return 0;
}

