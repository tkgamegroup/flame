#include <string>

typedef unsigned int		uint;

constexpr uint ch(char const* str)
{
	auto ret = std::_FNV_offset_basis;
	while (*str)
	{
		ret ^= *str;
		ret *= std::_FNV_prime;
		str++;
	}
	return ret;
}

enum eeee
{
	ABC,
	DEF
};

constexpr uint fuck(eeee a, const char* str)
{
	auto ret = ch(str);
	ret ^= a;
	ret *= std::_FNV_prime;
	return ret;
}

template <auto V> inline constexpr auto S = V;

inline void get(uint, eeee, const char*)
{

}

inline void get(eeee e, const char* n)
{
	fuck(e, n);

}

int main()
{
	auto a = S<fuck(DEF, "hello")>;
	auto b = S<ch("hello")>;
	auto c = ch("hello");
    return 0;
}
