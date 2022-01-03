#pragma once

#include <stdarg.h>
#include <assert.h>
#include <string>
#include <array>
#include <vector>
#include <deque>
#include <span>
#include <stack>
#include <list>
#include <map>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <random>
#include <filesystem>

#define FLAME_UNIQUE __FILE__, __LINE__

namespace flame
{
	typedef char*				charptr;
	typedef wchar_t*			wcharptr;
	typedef unsigned char		uchar;
	typedef unsigned short		ushort;
	typedef unsigned int		uint;
	typedef long long			int64;
	typedef unsigned long long  uint64;
	typedef void*				voidptr;

	const auto INVALID_POINTER = (void*)0x7fffffffffffffff;

	template<unsigned N>
	struct fixed_string
	{
		char buf[N + 1]{};

		constexpr fixed_string(const char(&str)[N])
		{
			for (unsigned i = 0; i != N; i++)
				buf[i] = str[i];
		}

		constexpr operator char const* () const { return buf; }
	};

	template<typename...>
	struct type_list {};

	template<typename U, typename T, typename... Args>
	constexpr bool is_one_of(type_list<T, Args...>)
	{
		return std::is_same_v<T, U> || is_one_of<U>(type_list<Args...>());
	}

	template<typename U, typename T>
	constexpr bool is_one_of(type_list<T>)
	{
		return std::is_same_v<T, U>;
	}

	template<class, template<typename...> class>
	inline constexpr bool is_specialization = false;

	template<template<typename...> typename T, typename... Args>
	inline constexpr bool is_specialization<T<Args...>, T> = true;

	template<typename T>
	concept int_type = std::signed_integral<T> && !std::same_as<T, int64>;

	template<typename T>
	concept uint_type = std::unsigned_integral<T> && !std::same_as<T, uint64>;

	template<typename T>
	concept int64_type = std::same_as<T, int64>;

	template<typename T>
	concept uint64_type = std::same_as<T, uint64>;

	template<typename T>
	concept enum_type = std::is_enum_v<T>;

	using basic_std_types = type_list<void, bool, char, uchar, wchar_t, short, ushort, int, uint, int64, uint64, float, std::string, std::wstring, std::filesystem::path>;

	template<typename T>
	concept basic_std_type = is_one_of<T>(basic_std_types());

	template<typename T>
	concept pointer_type = std::is_pointer_v<T>;

	template<typename T>
	concept vector_type = is_specialization<T, std::vector>;

	constexpr uint sh(char const* str)
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

	constexpr uint sh(const std::string_view& str)
	{
		auto ret = std::_FNV_offset_basis;
		for (auto sh : str)
		{
			ret ^= sh;
			ret *= std::_FNV_prime;
		}
		return ret;
	}

	consteval uint operator "" _h(char const* str, std::size_t len)
	{
		return sh(str);
	}

	template<typename T>
	consteval auto tn()
	{
		constexpr auto name = std::string_view{ __FUNCSIG__ };
		constexpr auto prefix1 = std::string_view{ " flame::tn<" };
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

	template<typename T>
	consteval unsigned int th()
	{
		return sh(tn<T>());
	}

	template<typename F>
	void* f2a(F f) // function to address
	{
		union
		{
			F f;
			void* p;
		}cvt;
		cvt.f = f;
		return cvt.p;
	}

	template<typename F>
	F a2f(void* p) // address to function
	{
		union
		{
			void* p;
			F f;
		}cvt;
		cvt.p = p;
		return cvt.f;
	}
}
