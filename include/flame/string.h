//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once

#include <flame/type.h>
#include <flame/memory.h>

#include <string>
#include <vector>
#include <codecvt>
#include <locale>
#include <sstream>
#include <regex>

inline constexpr unsigned int _HASH(char const * str, unsigned int seed)
{
	return 0 == *str ? seed : _HASH(str + 1, seed ^ (*str + 0x9e3779b9 + (seed << 6) + (seed >> 2)));
}

#define H(x) (_HASH(x, 0))

template <unsigned int N>
struct EnsureConst
{
	static const unsigned int value = N;
};

#define cH(x) (EnsureConst<_HASH(x, 0)>::value)

namespace flame
{
	template<typename CH>
	struct BasicString
	{
		int size;
		CH *v;

		inline void resize(int new_size)
		{
			if (size == new_size)
				return;

			size = new_size;
			v = (CH*)flame_realloc(v, sizeof(CH) * (size + 1));
			v[size] = (CH)0;
		}

		inline void set(const CH *s, int len = 0)
		{
			if (len == 0)
				len = std::char_traits<CH>::length(s);

			resize(len);
			memcpy(v, s, sizeof(CH) * size);
		}

		inline BasicString()
		{
			v = nullptr;
			resize(0);
		}

		inline BasicString(const BasicString &rhs) :
			BasicString()
		{
			set(rhs.v, rhs.size);
		}

		inline BasicString(BasicString &&rhs) :
			BasicString()
		{
			std::swap(size, rhs.size);
			std::swap(v, rhs.v);
		}

		inline BasicString(const std::basic_string<CH> &rhs) :
			BasicString()
		{
			set(rhs.c_str(), rhs.size());
		}

		inline BasicString &operator=(const BasicString &rhs)
		{
			set(rhs.v, rhs.size);

			return *this;
		}

		inline BasicString &operator=(BasicString &&rhs)
		{
			std::swap(size, rhs.size);
			std::swap(v, rhs.v);

			return *this;
		}

		inline BasicString &operator=(const CH *str)
		{
			set(str);

			return *this;
		}

		inline BasicString &operator=(const std::basic_string<CH> &rhs)
		{
			set(rhs.c_str(), rhs.size());

			return *this;
		}

		inline ~BasicString()
		{
			flame_free(v);
		}

		inline void insert(int pos, CH _v)
		{
			resize(size + 1);
			for (auto i = size - 1; i > pos; i--)
				v[i] = v[i - 1];
			v[pos] = _v;
		}

		inline void remove(int idx, int count = 1)
		{
			auto new_size = size - count;
			for (auto i = idx; i < new_size; i++)
				v[i] = v[i + count];
			resize(new_size);
		}

		inline int find(CH _v)
		{
			for (auto i = 0; i < size; i++)
			{
				if (v[i] == _v)
					return i;
			}
			return -1;
		}
	};

	template<typename CH>
	inline bool operator==(const BasicString<CH> &lhs, const char *rhs)
	{
		auto len = std::char_traits<CH>::length(rhs);
		return len == lhs.size && std::char_traits<CH>::compare(lhs.v, rhs, len) == 0;
	}

	template<typename CH>
	inline bool operator==(const char *lhs, const BasicString<CH> &rhs)
	{
		auto len = std::char_traits<CH>::length(lhs);
		return len == rhs.size && std::char_traits<CH>::compare(lhs, rhs.v, len) == 0;
	}

	template<typename CH>
	inline bool operator!=(const BasicString<CH> &lhs, const char *rhs)
	{
		return !(lhs == rhs);
	}

	template<typename CH>
	inline bool operator!=(const char *lhs, const BasicString<CH> &rhs)
	{
		return !(lhs == rhs);
	}

	template<typename CH>
	inline bool operator==(const BasicString<CH> &lhs, const BasicString<CH> &rhs)
	{
		return lhs.size == rhs.size && std::char_traits<CH>::compare(lhs.v, rhs.v, lhs.size) == 0;
	}

	template<typename CH>
	inline bool operator!=(const BasicString<CH> &lhs, const BasicString<CH> &rhs)
	{
		return !(lhs == rhs);
	}

	using String = BasicString<char>;
	using StringW = BasicString<wchar_t>;

	inline bool is_space_chr(int ch)
	{
		return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
	}

	inline bool is_slash_chr(int ch)
	{
		return ch == '\\' || ch == '/';
	}

	inline int get_str_line_number(const char *str)
	{
		int lineNumber = 0;
		while (*str)
		{
			if (*str == '\n')
				lineNumber++;
			str++;
		}
		return lineNumber;
	}

	template<typename CH>
	inline std::basic_string<CH> string_cut(const std::basic_string<CH> &str, int length)
	{
		if (length < 0)
			length = str.size() + length;
		return std::basic_string<CH>(str.begin(), str.begin() + length);
	}

	template<typename CH>
	inline std::vector<std::basic_string<CH>> string_split(const std::basic_string<CH> &str, CH delimiter = ' ')
	{
		std::basic_istringstream<CH> iss(str);
		std::vector<std::basic_string<CH>> ret;

		std::basic_string<CH> s;
		while (std::getline(iss, s, delimiter))
			ret.push_back(s);

		return ret;
	}

	template<typename CH>
	inline std::vector<std::basic_string<CH>> doublenull_string_split(const CH *str)
	{
		std::vector<std::basic_string<CH>> ret;

		auto p = str, q = str;
		while (true)
		{
			if (*p == 0)
			{
				ret.push_back(q);
				p++;
				if (*p == 0)
					break;
				q = p;
			}
			else
				p++;
		}

		return ret;
	}

	template<typename CH>
	inline std::vector<std::basic_string<CH>> string_regex_split(const std::basic_string<CH> &str, const std::basic_string<CH> &regex, int req_idx = 0)
	{
		std::vector<std::basic_string<CH>> ret;

		std::basic_regex<CH> reg(regex);
		std::match_results<typename std::basic_string<CH>::const_iterator> match;
		auto s = str;

		while (std::regex_search(s, match, reg))
		{
			ret.push_back(match[req_idx]);
			s = match.suffix();
		}

		return ret;
	}

	inline std::wstring a2w(const std::string &str)
	{
		setlocale(LC_ALL, "chs");
		auto len = mbstowcs(nullptr, str.c_str(), 0) + 1;
		std::wstring wstr;
		wstr.resize(len);
		mbstowcs((wchar_t*)wstr.data(), str.c_str(), len);
		setlocale(LC_ALL, "");
		return wstr;
	}

	inline std::wstring s2w(const std::string &str)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(str);
	}

	inline std::string w2s(const std::wstring &wstr)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes(wstr);
	}

#ifdef FLAME_WINDOWS
	inline std::string translate(const char *src_locale, const char *dst_locale, const std::string &src)
	{
		std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>> 
			cv1(new std::codecvt_byname<wchar_t, char, mbstate_t>(src_locale));
		std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>> 
			cv2(new std::codecvt_byname<wchar_t, char, mbstate_t>(dst_locale));
		return cv2.to_bytes(cv1.from_bytes(src));
	}

	inline std::string japanese_to_chinese(const std::string &src)
	{
		return translate(".932", ".936", src);
	}
#endif

	inline int base85_length(int plain_length)
	{
		return ((plain_length + 3) / 4) * 5;
	}

	inline uchar encode_base85_char(uint v)
	{
		v = (v % 85) + 35;
		return (v >= ']') ? v + 1 : v;
	}

	inline void encode_base85(const uchar *src, int src_length, uchar *dst)
	{
		for (auto i = 0; i < src_length; i += 4)
		{
			unsigned int d = *(uint*)(src + i);
			for (auto j = 0; j < 5; j++)
			{
				*dst = encode_base85_char(d);
				dst++;
				d /= 85;
			}
		}
	}

	inline uchar decode_base85_char(uchar c)
	{ 
		return c >= ']' ? c - 36 : c - 35; 
	}

	inline void decode_base85(const uchar *src, uchar *dst, int dst_length)
	{
		auto dst_off = 0;
		while (*src)
		{
			uint v = decode_base85_char(src[0]) +
					85 * (decode_base85_char(src[1]) +
					85 * (decode_base85_char(src[2]) +
					85 * (decode_base85_char(src[3]) +
					85 * decode_base85_char(src[4]))));
			for (auto i = 0; i < 4; i++)
			{
				if (dst_off + i < dst_length)
					dst[dst_off + i] = v & 0xff;
				v = (v >> 8);
			}
			src += 5;
			dst_off += 4;
		}
	}
}
