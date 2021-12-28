#include <string>

template <typename T>
consteval auto type_name()
{
	constexpr auto name = std::string_view{ __FUNCSIG__ };
	constexpr auto prefix1 = std::string_view{ " type_name<" };
	constexpr auto prefix2 = std::string_view{ "class " };
	constexpr auto prefix3 = std::string_view{ "struct " };
	constexpr auto suffix = std::string_view{ ">(void)" };

	auto start = name.find(prefix1) + prefix1.size();
	if (name.compare(start, prefix2.size(), prefix2) == 0)
		start += prefix2.size();
	else if (name.compare(start, prefix3.size(), prefix3) == 0)
		start += prefix3.size();
	auto end = name.rfind(suffix);

	return name.substr(start, (end - start));
}

template<auto V> inline constexpr auto S = V;

constexpr unsigned int ch(const std::string_view& str)
{
	auto ret = std::_FNV_offset_basis;
	for (auto ch : str)
	{
		ret ^= ch;
		ret *= std::_FNV_prime;
	}
	return ret;
}

template<typename T>
constexpr unsigned int type_hash()
{
	return S<ch(type_name<T>())>;
}

struct A
{

};

#include <iostream>

int main()
{
	auto wtf = type_hash<A>();
	ch("A");
	auto a = type_name<A>();
	return 0;
}
