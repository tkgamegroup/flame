#include <string>
#include <sstream>
#include <vector>

template <class T>
concept concept_int64 = std::signed_integral<T> && sizeof(T) == 8;

template <class T>
concept concept_uint64 = std::unsigned_integral<T> && sizeof(T) == 8;

template <std::signed_integral T, class CH>
inline T sto(const std::basic_string<CH>& s)
{
	T ret;
	try { ret = std::stoi(s); }
	catch (...) { ret = 0; }
	return ret;
}

template <std::signed_integral T, class CH>
inline T sto(const CH* s)
{
	return sto<T, CH>(std::basic_string<CH>(s));
}

template <std::unsigned_integral T, class STR>
inline T sto(const STR& s)
{
	T ret;
	try { ret = std::stoul(s); }
	catch (...) { ret = 0; }
	return ret;
}

template <concept_int64 T, class STR>
inline T sto(const STR& s)
{
	T ret;
	try { ret = std::stoll(s); }
	catch (...) { ret = 0; }
	return ret;
}

template <concept_uint64 T, class STR>
inline T sto(const STR& s)
{
	T ret;
	try { ret = std::stoull(s); }
	catch (...) { ret = 0; }
	return ret;
}

template <unsigned int N, class T>
struct Vec
{
	T v[N];
};

template <unsigned int N, std::signed_integral T, class STR>
inline Vec<N, T> sto(const STR& s)
{
	Vec<N, T> ret;
	return ret;
}

int main()
{
	auto a = sto<unsigned int>("123");
	auto b = sto<int>("456");
	auto c = sto<long long>(L"123456789123456");
	auto d = sto<unsigned long long>(std::string("123456789123456"));

	return 0;
}
