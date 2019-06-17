#pragma once

#ifdef FLAME_WINDOWS
#ifdef FLAME_FOUNDATION_MODULE
#define FLAME_FOUNDATION_EXPORTS __declspec(dllexport)
#else
#define FLAME_FOUNDATION_EXPORTS __declspec(dllimport)
#endif
#else
#define FLAME_FOUNDATION_EXPORTS
#endif

#include <flame/math.h>

#include <memory>
#include <vector>
#include <list>
#include <map>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <locale>
#include <codecvt>
#include <regex>
#include <fstream>
#include <experimental/filesystem>
#include <mutex>
#include <stdarg.h>
#include <assert.h>

#ifdef FLAME_WINDOWS
#define LOGI(...) {printf(__VA_ARGS__);printf("\n");}
#define LOGW(...) {printf(__VA_ARGS__);printf("\n");}
#elif FLAME_ANDROID
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "flame", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "flame", __VA_ARGS__))
#endif

namespace std
{
	namespace fs = std::experimental::filesystem;
}

FLAME_FOUNDATION_EXPORTS void *flame_malloc(int size);
FLAME_FOUNDATION_EXPORTS void *flame_realloc(void *p, int size);
FLAME_FOUNDATION_EXPORTS void flame_free(void *p);

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
	enum KeyState
	{
		KeyStateNull,
		KeyStateUp = 1 << 0,
		KeyStateDown = 1 << 1,
		KeyStateJust = 1 << 2,
		KeyStateDouble = 1 << 2,
	};

	enum Key
	{
		Key_Null = -1,
		Key_Backspace,
		Key_Tab,
		Key_Enter,
		Key_Shift,
		Key_Ctrl,
		Key_Alt,
		Key_Pause,
		Key_CapsLock,
		Key_Esc,
		Key_Space,
		Key_PgUp, Key_PgDn,
		Key_End,
		Key_Home,
		Key_Left, Key_Up, Key_Right, Key_Down,
		Key_PrtSc,
		Key_Ins,
		Key_Del,
		Key_0, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9,
		Key_A, Key_B, Key_C, Key_D, Key_E, Key_F, Key_G, Key_H, Key_I, Key_J, Key_K, Key_L, Key_M, Key_N, Key_O, Key_P, Key_Q, Key_R, Key_S, Key_T, Key_U, Key_V, Key_W, Key_X, Key_Y, Key_Z,
		Key_Numpad0, Key_Numpad1, Key_Numpad2, Key_Numpad3, Key_Numpad4, Key_Numpad5, Key_Numpad6, Key_Numpad7, Key_Numpad8, Key_Numpad9,
		Key_Add, Key_Subtract, Key_Multiply, Key_Divide,
		Key_Separator,
		Key_Decimal,
		Key_F1, Key_F2, Key_F3, Key_F4, Key_F5, Key_F6, Key_F7, Key_F8, Key_F9, Key_F10, Key_F11, Key_F12,
		Key_NumLock,
		Key_ScrollLock,

		Key_count,

		KeyMax = 0xffffffff
	};

	enum MouseKey
	{
		Mouse_Null = -1,
		Mouse_Left,
		Mouse_Right,
		Mouse_Middle,

		MouseKey_count
	};

	enum FocusType
	{
		Focus_Gain,
		Focus_Lost
	};

	inline bool is_key_down(KeyState action) // value is Key
	{
		return action == KeyStateDown;
	}

	inline bool is_key_up(KeyState action) // value is Key
	{
		return action == KeyStateUp;
	}

	inline bool is_key_char(KeyState action) // value is ch
	{
		return action == KeyStateNull;
	}

	inline bool is_mouse_enter(KeyState action, MouseKey key)
	{ 
		return action == KeyStateDown && key == Mouse_Null; 
	}

	inline bool is_mouse_leave(KeyState action, MouseKey key)
	{ 
		return action == KeyStateUp && key == Mouse_Null; 
	}

	inline bool is_mouse_down(KeyState action, MouseKey key, bool just = false) // value is pos
	{ 
		return action == (KeyStateDown | (just ? KeyStateJust : 0)) && key != Mouse_Null; 
	}

	inline bool is_mouse_up(KeyState action, MouseKey key, bool just = false) // value is pos
	{ 
		return action == (KeyStateUp | (just ? KeyStateJust : 0)) && key != Mouse_Null;
	}

	inline bool is_mouse_move(KeyState action, MouseKey key) // value is disp
	{ 
		return action == KeyStateNull && key == Mouse_Null; 
	}

	inline bool is_mouse_scroll(KeyState action, MouseKey key) // value.x() is scroll value
	{ 
		return action == KeyStateNull && key == Mouse_Middle; 
	}

	inline bool is_mouse_clicked(KeyState action, MouseKey key, bool db = false)
	{ 
		return action == (KeyStateDown | KeyStateUp | (db ? KeyStateDouble : 0)) && key == Mouse_Null;
	}

	inline ulonglong get_now_ns()
	{
		return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	}

	template<typename T>
	struct Array
	{
		uint size;
		T* v;
	};

	template<typename T>
	struct DynamicArray
	{
		uint size;
		T* v;

		void _init(const T* p, uint len)
		{
			size = len;
			v = (T*)flame_malloc(sizeof(T) * size);
			for (auto i = 0; i < size; i++)
				new(&v[i])T(p[i]);
		}

		DynamicArray()
		{
			size = 0;
			v = nullptr;
		}

		DynamicArray(const DynamicArray& rhs)
		{
			size = rhs.size;
			v = (T*)flame_malloc(sizeof(T) * size);
			for (auto i = 0; i < size; i++)
				new(&v[i])T(rhs[i]);
		}

		DynamicArray(DynamicArray&& rhs)
		{
			size = rhs.size;
			v = rhs.v;

			rhs.size = 0;
			rhs.v = nullptr;
		}

		~DynamicArray()
		{
			for (auto i = 0; i < size; i++)
				v[i].~T();
			flame_free(v);
		}

		DynamicArray& operator=(const DynamicArray& rhs)
		{
			assign(rhs.v, rhs.size);
		}

		DynamicArray& operator=(DynamicArray&& rhs)
		{
			std::swap(size, rhs.size);
			std::swap(v, rhs.v);

			return *this;
		}

		DynamicArray& operator=(const std::vector<T>& rhs)
		{
			assign(rhs.data(), rhs.size());

			return *this;
		}

		T &operator[](uint idx)
		{
			return v[idx];
		}

		const T &operator[](uint idx) const
		{
			return v[idx];
		}

		void assign(const T* p, uint len)
		{
			for (auto i = 0; i < size; i++)
				v[i].~T();

			size = len;

			v = (T*)flame_realloc(v, sizeof(T) * size);

			for (auto i = 0; i < size; i++)
				new(&p[i])T();
		}

		void resize(uint new_size, uint type_size = 0 /* for pod that is not T type */)
		{
			if (size == new_size)
				return;
			if (type_size == 0)
				type_size = sizeof(T);

			if (size > new_size)
			{
				for (auto i = new_size; i < size; i++)
					v[i].~T();
			}

			if (new_size == 0)
			{
				flame_free(v);
				v = nullptr;
			}
			else
				v = (T*)flame_realloc(v, type_size * new_size);

			if (new_size > size)
			{
				for (auto i = size; i < new_size; i++)
					new(&v[i])T();
			}

			size = new_size;
		}

		void insert(uint pos, const T &_v)
		{
			resize(size + 1);
			for (auto i = size - 1; i > pos; i--)
				v[i] = std::move(v[i - 1]);
			v[pos] = _v;
		}

		void push_back(const T &_v)
		{
			insert(size, _v);
		}

		void remove(uint idx, uint count = 1)
		{
			auto new_size = size - count;
			for (auto i = idx; i < new_size; i++)
				v[i] = std::move(v[i + count]);
			resize(new_size);
		}

		int find(const T &_v)
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
	struct BasicString
	{
		uint size;
		CH *v;

		void _init(const CH* s, uint len = 0)
		{
			if (len == 0)
				len = std::char_traits<CH>::length(s);

			size = len;
			v = (CH*)flame_malloc(sizeof(CH) * (size + 1));
			memcpy(v, s, size);
			v[size] = (CH)0;
		}

		BasicString()
		{
			size = 0;
			v = (CH*)flame_malloc(sizeof(CH));
			v[size] = (CH)0;
		}

		BasicString(const BasicString& rhs)
		{
			_init(rhs.v, rhs.size);
		}

		BasicString(BasicString&& rhs)
		{
			size = rhs.size;
			v = rhs.v;

			rhs.size = 0;
			rhs.v = nullptr;
		}

		BasicString(const CH* rhs)
		{
			_init(rhs);
		}

		BasicString(const std::basic_string<CH>& rhs)
		{
			_init(rhs.c_str(), rhs.size());
		}

		~BasicString()
		{
			flame_free(v);
		}

		BasicString& operator=(const BasicString& rhs)
		{
			assign(rhs.v, rhs.size);

			return *this;
		}

		BasicString& operator=(BasicString&& rhs)
		{
			std::swap(size, rhs.size);
			std::swap(v, rhs.v);

			return *this;
		}

		BasicString& operator=(const CH *str)
		{
			assign(str);

			return *this;
		}

		BasicString& operator=(const std::basic_string<CH>& rhs)
		{
			assign(rhs.c_str(), rhs.size());

			return *this;
		}

		void assign(const CH* s, uint len = 0)
		{
			if (len == 0)
				len = std::char_traits<CH>::length(s);

			resize(len);
			memcpy(v, s, sizeof(CH) * size);
		}

		void resize(uint new_size)
		{
			if (size == new_size)
				return;

			size = new_size;
			v = (CH*)flame_realloc(v, sizeof(CH) * (size + 1));
			v[size] = (CH)0;
		}

		void insert(uint pos, CH _v)
		{
			resize(size + 1);
			for (auto i = size - 1; i > pos; i--)
				v[i] = v[i - 1];
			v[pos] = _v;
		}

		void remove(uint idx, uint count = 1)
		{
			auto new_size = size - count;
			for (auto i = idx; i < new_size; i++)
				v[i] = v[i + count];
			resize(new_size);
		}

		uint find(CH _v)
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

	struct StringAndHash : String
	{
		uint hash;

		void assign(const char *s, uint len = 0)
		{
			String::assign(s, len);
			hash = v ? H(v) : 0;
		}

		StringAndHash()
		{
			hash = 0;
		}

		StringAndHash(const StringAndHash &rhs) :
			StringAndHash()
		{
			assign(rhs.v, rhs.size);
		}

		StringAndHash(StringAndHash &&rhs) :
			StringAndHash()
		{
			std::swap(size, rhs.size);
			std::swap(v, rhs.v);
			std::swap(hash, rhs.hash);
		}

		StringAndHash(const std::string &rhs) :
			StringAndHash()
		{
			assign(rhs.c_str(), rhs.size());
		}

		StringAndHash &operator=(const StringAndHash &rhs)
		{
			assign(rhs.v, rhs.size);

			return *this;
		}

		StringAndHash &operator=(StringAndHash &&rhs)
		{
			std::swap(size, rhs.size);
			std::swap(v, rhs.v);
			std::swap(hash, rhs.hash);

			return *this;
		}

		StringAndHash &operator=(const char *str)
		{
			assign(str);

			return *this;
		}

		StringAndHash &operator=(const std::string &rhs)
		{
			assign(rhs.c_str(), rhs.size());

			return *this;
		}
	};

	inline bool is_space_chr(int ch)
	{
		return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
	}

	inline bool is_slash_chr(int ch)
	{
		return ch == '\\' || ch == '/';
	}

	inline uint get_str_line_number(const char *str)
	{
		auto lineNumber = 0u;
		while (*str)
		{
			if (*str == '\n')
				lineNumber++;
			str++;
		}
		return lineNumber;
	}

	template<typename CH>
	inline std::basic_string<CH> string_cut(const std::basic_string<CH> &str, int length) // < 0 means from end
	{
		if (length < 0)
			length = str.size() + length;
		return std::basic_string<CH>(str.begin(), str.begin() + length);
	}

	template<typename CH>
	inline void string_to_lower(std::basic_string<CH> &str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	}

	template<typename CH>
	inline void string_to_upper(std::basic_string<CH> &str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::toupper);
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
	inline std::vector<std::basic_string<CH>> string_regex_split(const std::basic_string<CH> &str, const std::basic_string<CH> &regex, uint req_idx = 0)
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

	// baseXX encoding is from https://github.com/r-lyeh-archived/base

	inline std::string base85_encode(const uchar* data, uint length)
	{
		assert(length % 4 == 0);

		const char encoder[86] =
			"0123456789" "abcdefghij" "klmnopqrst" "uvwxyzABCD"             // 00..39
			"EFGHIJKLMN" "OPQRSTUVWX" "YZ.-:+=^!/" "*?&<>()[]{" "}@%$#";    // 40..84 // free chars: , ; _ ` | ~ \'

		std::string ret;
		ret.resize(((length + 3) / 4) * 5);
		auto dst = &ret[0];

		for (auto i = 0; i < length; i += 4)
		{
			auto d = *(uint*)(data + i);
			for (auto j = 0; j < 5; j++)
			{
				*dst = encoder[d];
				dst++;
				d /= 85;
			}
		}

		return ret;
	}

	inline void base85_decode(const std::string& str, uchar *dst) // dst length is str length * 4 / 5
	{
		assert(str.length() == 5);

		const unsigned char decoder[128] = {
				0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x00..0x0F
				0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x10..0x1F
				0, 68,  0, 84, 83, 82, 72,  0, 75, 76, 70, 65,  0, 63, 62, 69, // 0x20..0x2F
				0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 64,  0, 73, 66, 74, 71, // 0x30..0x3F
			   81, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, // 0x40..0x4F
			   51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77,  0, 78, 67,  0, // 0x50..0x5F
				0, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, // 0x60..0x6F
			   25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 79,  0, 80,  0,  0, // 0x70..0x7F
		};

		auto src = &str[0];

		auto dst_off = 0;
		while (*src)
		{
			uint v = decoder[src[0]] +
				85 * (decoder[src[1]] +
					85 * (decoder[src[2]] +
						85 * (decoder[src[3]] +
							85 * decoder[src[4]])));
			for (auto i = 0; i < 4; i++)
			{
				dst[dst_off + i] = v & 0xff;
				v = (v >> 8);
			}
			src += 5;
			dst_off += 4;
		}
	}

	inline std::string base64_encode(const std::string& text)
	{
		std::string out;

		const std::string chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";

		unsigned char const* bytes_to_encode = (unsigned char const*)text.c_str();
		unsigned int in_len = (unsigned int)text.size();
		unsigned int i = 0;
		unsigned int j = 0;
		unsigned char char_array_3[3];
		unsigned char char_array_4[4];

		while (in_len--) {
			char_array_3[i++] = *(bytes_to_encode++);
			if (i == 3) {
				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				for (i = 0; (i < 4); i++)
					out += chars[char_array_4[i]];
				i = 0;
			}
		}

		if (i) {
			for (j = i; j < 3; j++)
				char_array_3[j] = '\0';

			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (j = 0; (j < i + 1); j++)
				out += chars[char_array_4[j]];

			while ((i++ < 3))
				out += '=';
		}

		return out;
	}

	inline std::string base64_decode(const std::string& encoded)
	{
		std::string out;

		const std::string chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";

		unsigned int in_len = (unsigned int)encoded.size();
		unsigned int i = 0;
		unsigned int j = 0;
		unsigned int in_ = 0;
		unsigned char char_array_4[4], char_array_3[3];
		out.clear();

		while (in_len-- && (encoded[in_] != '=') && //is_base64(encoded[in_])) {
			(isalnum(encoded[in_]) || encoded[in_] == '+' || encoded[in_] == '/')) {
			char_array_4[i++] = encoded[in_]; in_++;
			if (i == 4) {
				for (i = 0; i < 4; i++)
					char_array_4[i] = chars.find(char_array_4[i]);

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				for (i = 0; (i < 3); i++)
					out += char_array_3[i];
				i = 0;
			}
		}

		if (i) {
			for (j = i; j < 4; j++)
				char_array_4[j] = 0;

			for (j = 0; j < 4; j++)
				char_array_4[j] = chars.find(char_array_4[j]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (j = 0; (j < i - 1); j++) out += char_array_3[j];
		}

		return out;
	}

	// SHA1 is from https://github.com/vog/sha1

	struct SHA1
	{
		uint digest[5];
		std::string buffer;
		ulonglong transforms; 

		enum { BLOCK_INTS = 16, BLOCK_BYTES = BLOCK_INTS * 4 };

		SHA1() 
		{
			reset();
		}

		void reset()
		{
			digest[0] = 0x67452301;
			digest[1] = 0xefcdab89;
			digest[2] = 0x98badcfe;
			digest[3] = 0x10325476;
			digest[4] = 0xc3d2e1f0;

			buffer = "";
			transforms = 0;
		}

		void update(const std::string& s)
		{
			std::istringstream is(s);
			while (true)
			{
				char sbuf[BLOCK_BYTES];
				is.read(sbuf, BLOCK_BYTES - buffer.size());
				buffer.append(sbuf, (std::size_t)is.gcount());
				if (buffer.size() != BLOCK_BYTES)
				{
					return;
				}
				uint block[BLOCK_INTS];
				buffer_to_block(buffer, block);
				transform(digest, block, transforms);
				buffer.clear();
			}
		}

		void _final()
		{
			ulonglong total_bits = (transforms * BLOCK_BYTES + buffer.size()) * 8;

			buffer += (char)0x80;
			size_t orig_size = buffer.size();
			while (buffer.size() < BLOCK_BYTES)
				buffer += (char)0x00;

			uint block[BLOCK_INTS];
			buffer_to_block(buffer, block);

			if (orig_size > BLOCK_BYTES - 8)
			{
				transform(digest, block, transforms);
				for (size_t i = 0; i < BLOCK_INTS - 2; i++)
					block[i] = 0;
			}

			block[BLOCK_INTS - 1] = (uint)total_bits;
			block[BLOCK_INTS - 2] = (uint)(total_bits >> 32);
			transform(digest, block, transforms);
		}

		std::string final_bin()
		{
			_final();

			std::string result;
			result.resize(sizeof(uint) * FLAME_ARRAYSIZE(digest));
			auto dst = &result[0];
			for (auto i = 0; i < FLAME_ARRAYSIZE(digest); i++)
			{
				for (auto j = 0; j < 4; j++)
					dst[j] = ((char*)(&digest[i]))[4 - j - 1];
				dst += 4;
			}

			reset();

			return result;
		}

		std::string final_str()
		{
			_final();

			std::ostringstream result;
			for (auto i = 0; i < FLAME_ARRAYSIZE(digest); i++)
			{
				result << std::hex << std::setfill('0') << std::setw(8);
				result << digest[i];
			}

			reset();

			return result.str();
		}

		static uint rol(const uint value, const size_t bits)
		{
			return (value << bits) | (value >> (32 - bits));
		}


		static uint blk(const uint block[BLOCK_INTS], const size_t i)
		{
			return rol(block[(i + 13) & 15] ^ block[(i + 8) & 15] ^ block[(i + 2) & 15] ^ block[i], 1);
		}

		static void R0(const uint block[BLOCK_INTS], const uint v, uint& w, const uint x, const uint y, uint& z, const size_t i)
		{
			z += ((w & (x ^ y)) ^ y) + block[i] + 0x5a827999 + rol(v, 5);
			w = rol(w, 30);
		}

		static void R1(uint block[BLOCK_INTS], const uint v, uint& w, const uint x, const uint y, uint& z, const size_t i)
		{
			block[i] = blk(block, i);
			z += ((w & (x ^ y)) ^ y) + block[i] + 0x5a827999 + rol(v, 5);
			w = rol(w, 30);
		}

		static void R2(uint block[BLOCK_INTS], const uint v, uint& w, const uint x, const uint y, uint& z, const size_t i)
		{
			block[i] = blk(block, i);
			z += (w ^ x ^ y) + block[i] + 0x6ed9eba1 + rol(v, 5);
			w = rol(w, 30);
		}

		static void R3(uint block[BLOCK_INTS], const uint v, uint& w, const uint x, const uint y, uint& z, const size_t i)
		{
			block[i] = blk(block, i);
			z += (((w | x) & y) | (w & x)) + block[i] + 0x8f1bbcdc + rol(v, 5);
			w = rol(w, 30);
		}

		static void R4(uint block[BLOCK_INTS], const uint v, uint & w, const uint x, const uint y, uint & z, const size_t i)
		{
			block[i] = blk(block, i);
			z += (w ^ x ^ y) + block[i] + 0xca62c1d6 + rol(v, 5);
			w = rol(w, 30);
		}

		static void transform(uint digest[], uint block[BLOCK_INTS], ulonglong& transforms)
		{
			uint a = digest[0];
			uint b = digest[1];
			uint c = digest[2];
			uint d = digest[3];
			uint e = digest[4];

			R0(block, a, b, c, d, e, 0);
			R0(block, e, a, b, c, d, 1);
			R0(block, d, e, a, b, c, 2);
			R0(block, c, d, e, a, b, 3);
			R0(block, b, c, d, e, a, 4);
			R0(block, a, b, c, d, e, 5);
			R0(block, e, a, b, c, d, 6);
			R0(block, d, e, a, b, c, 7);
			R0(block, c, d, e, a, b, 8);
			R0(block, b, c, d, e, a, 9);
			R0(block, a, b, c, d, e, 10);
			R0(block, e, a, b, c, d, 11);
			R0(block, d, e, a, b, c, 12);
			R0(block, c, d, e, a, b, 13);
			R0(block, b, c, d, e, a, 14);
			R0(block, a, b, c, d, e, 15);
			R1(block, e, a, b, c, d, 0);
			R1(block, d, e, a, b, c, 1);
			R1(block, c, d, e, a, b, 2);
			R1(block, b, c, d, e, a, 3);
			R2(block, a, b, c, d, e, 4);
			R2(block, e, a, b, c, d, 5);
			R2(block, d, e, a, b, c, 6);
			R2(block, c, d, e, a, b, 7);
			R2(block, b, c, d, e, a, 8);
			R2(block, a, b, c, d, e, 9);
			R2(block, e, a, b, c, d, 10);
			R2(block, d, e, a, b, c, 11);
			R2(block, c, d, e, a, b, 12);
			R2(block, b, c, d, e, a, 13);
			R2(block, a, b, c, d, e, 14);
			R2(block, e, a, b, c, d, 15);
			R2(block, d, e, a, b, c, 0);
			R2(block, c, d, e, a, b, 1);
			R2(block, b, c, d, e, a, 2);
			R2(block, a, b, c, d, e, 3);
			R2(block, e, a, b, c, d, 4);
			R2(block, d, e, a, b, c, 5);
			R2(block, c, d, e, a, b, 6);
			R2(block, b, c, d, e, a, 7);
			R3(block, a, b, c, d, e, 8);
			R3(block, e, a, b, c, d, 9);
			R3(block, d, e, a, b, c, 10);
			R3(block, c, d, e, a, b, 11);
			R3(block, b, c, d, e, a, 12);
			R3(block, a, b, c, d, e, 13);
			R3(block, e, a, b, c, d, 14);
			R3(block, d, e, a, b, c, 15);
			R3(block, c, d, e, a, b, 0);
			R3(block, b, c, d, e, a, 1);
			R3(block, a, b, c, d, e, 2);
			R3(block, e, a, b, c, d, 3);
			R3(block, d, e, a, b, c, 4);
			R3(block, c, d, e, a, b, 5);
			R3(block, b, c, d, e, a, 6);
			R3(block, a, b, c, d, e, 7);
			R3(block, e, a, b, c, d, 8);
			R3(block, d, e, a, b, c, 9);
			R3(block, c, d, e, a, b, 10);
			R3(block, b, c, d, e, a, 11);
			R4(block, a, b, c, d, e, 12);
			R4(block, e, a, b, c, d, 13);
			R4(block, d, e, a, b, c, 14);
			R4(block, c, d, e, a, b, 15);
			R4(block, b, c, d, e, a, 0);
			R4(block, a, b, c, d, e, 1);
			R4(block, e, a, b, c, d, 2);
			R4(block, d, e, a, b, c, 3);
			R4(block, c, d, e, a, b, 4);
			R4(block, b, c, d, e, a, 5);
			R4(block, a, b, c, d, e, 6);
			R4(block, e, a, b, c, d, 7);
			R4(block, d, e, a, b, c, 8);
			R4(block, c, d, e, a, b, 9);
			R4(block, b, c, d, e, a, 10);
			R4(block, a, b, c, d, e, 11);
			R4(block, e, a, b, c, d, 12);
			R4(block, d, e, a, b, c, 13);
			R4(block, c, d, e, a, b, 14);
			R4(block, b, c, d, e, a, 15);

			digest[0] += a;
			digest[1] += b;
			digest[2] += c;
			digest[3] += d;
			digest[4] += e;

			transforms++;
		}

		static void buffer_to_block(const std::string& buffer, uint block[BLOCK_INTS])
		{
			for (size_t i = 0; i < BLOCK_INTS; i++)
			{
				block[i] = (buffer[4 * i + 3] & 0xff)
					| (buffer[4 * i + 2] & 0xff) << 8
					| (buffer[4 * i + 1] & 0xff) << 16
					| (buffer[4 * i + 0] & 0xff) << 24;
			}
		}
	};

	template<class F>
	struct Function
	{
		F* f;
		uint c_size;
		void* c;
		uint c_hash;

		void _init(F _f, uint _c_size, void* _c, uint _c_hash = 0) // each of the captures must be pod or BasicString<char|wchar_t> or Array<POD>
		{
			f = _f;
			c_size = _c_size;
			if (c_size > 0)
			{
				c = flame_malloc(c_size);
				memcpy(c, _c, c_size);
			}
			else
				c = nullptr;
			c_hash = _c_hash;
		}

		void assign(F _f, uint _c_size, void* _c, uint _c_hash = 0)
		{
			flame_free(c);
			f = _f;
			c_size = _c_size;
			if (c_size > 0)
			{
				c = flame_malloc(c_size);
				memcpy(c, _c, c_size);
			}
			else
				c = nullptr;
			c_hash = _c_hash;
		}

		Function()
		{
			f = nullptr;
			c_size = 0;
			c = nullptr;
			c_hash = 0;
		}

		Function(F f, uint c_size = 0, void* c = 0, uint c_hash = 0)
		{
			_init(f, c_size, c, c_hash);
		}

		Function(const Function<F>& rhs)
		{
			_init(rhs.f, rhs.c_size, rhs.c, rhs.c_hash);
		}

		Function& operator=(const Function& rhs)
		{
			assign(rhs.f, rhs.c_size, rhs.c, rhs.c_hash);

			return *this;
		}

		Function& operator=(Function&& rhs)
		{
			std::swap(f, rhs.f);
			std::swap(c_size, rhs.c_size);
			std::swap(c, rhs.c);
			std::swap(c_hash, rhs.c_hash);

			return *this;
		}

		template<class ...Args>
		auto operator()(Args... args)
		{
			return f(c, args...);
		}
	};

	FLAME_FOUNDATION_EXPORTS void thread(const Function<void(void* c)>& f);

	inline std::fs::path ext_replace(const std::fs::path& path, const std::wstring& ext)
	{
		if (path.extension().wstring() != ext)
		{
			auto pp = path.parent_path().wstring();
			if (pp != L"")
				pp += L"\\";
			return pp + path.stem().wstring() + ext;
		}
		return path;
	}

	template<class T>
	inline T read(std::ifstream &file)
	{
		T v;
		file.read((char*)&v, sizeof(T));
		return v;
	}

	inline std::string read_string(std::ifstream &file)
	{
		uint size = 0;
		uint q = 1;
		for (auto i = 0; i < 4; i++)
		{
			unsigned char byte;
			file.read((char*)&byte, 1);
			if (byte >= 128)
				byte -= 128;
			else
				i = 4;
			size += q * byte;
			q *= 128;
		}
		std::string v;
		v.resize(size);
		file.read((char*)v.data(), size);
		return v;
	}

	template<class T>
	inline void write(std::ofstream &file, const T &v)
	{
		file.write((char*)&v, sizeof(T));
	}

	inline void write_string(std::ofstream &file, const std::string &v)
	{
		uint size = v.size();
		for (auto i = 0; i < 4; i++)
		{
			unsigned char byte = size % 128;
			size /= 128;
			if (size > 0)
				byte += 128;
			else
				i = 4;
			file.write((char*)&byte, 1);

		}
		file.write((char*)v.data(), v.size());
	}

	inline void write_fmt(std::ofstream &file, const char *fmt, ...)
	{
		static char buffer[1024];

		va_list ap;
		va_start(ap, fmt);
		auto len = vsprintf(buffer, fmt, ap);
		va_end(ap);

		file.write(buffer, len);
	}

	enum FileType
	{
		FileTypeUnknown,
		FileTypeFolder,
		FileTypeText,
		FileTypeImage,
		FileTypeModel
	};

	inline bool is_text_file(const std::wstring &_ext)
	{
		auto ext = _ext;
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext == L".txt" ||
			ext == L".h" || ext == L".c" || ext == L".cpp" || ext == L".hpp" || ext == L".cxx" || ext == L".inl" ||
			ext == L".glsl" || ext == L".vert" || ext == L".tesc" || ext == L".tese" || ext == L".geom" || ext == L".frag" || ext == L".hlsl" ||
			ext == L".xml" || ext == L".json" || ext == L".ini" || ext == L".log" ||
			ext == L".htm" || ext == L".html" || ext == L".css" ||
			ext == L".sln" || ext == L".vcxproj")
			return true;
		return false;
	}

	inline bool is_image_file(const std::wstring &_ext)
	{
		auto ext = _ext;
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext == L".bmp" || ext == L".jpg" || ext == L".jpeg" || ext == L".png" || ext == L".gif" ||
			ext == L".tga" || ext == L".dds" || ext == L".ktx")
			return true;
		return false;
	}

	inline bool is_model_file(const std::wstring &_ext)
	{
		auto ext = _ext;
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext == L".obj" || ext == L".pmd" || ext == L".pmx" || ext == L".dae")
			return true;
		return false;
	}

	inline FileType get_file_type(const std::wstring &ext)
	{
		if (is_text_file(ext))
			return FileTypeText;
		if (is_image_file(ext))
			return FileTypeImage;
		if (is_model_file(ext))
			return FileTypeModel;
		return FileTypeUnknown;
	}

	inline long long get_file_length(std::ifstream &f)
	{
		f.seekg(0, std::ios::end);
		auto s = f.tellg();
		f.seekg(0, std::ios::beg);
		return s;
	}

	inline std::pair<std::unique_ptr<char[]>, long long> get_file_content(const std::wstring &filename)
	{
#ifdef FLAME_WINDOWS
		std::ifstream file(filename, std::ios::binary);
#else
		auto utf8_filename = w2s(filename);
		std::ifstream file(utf8_filename, std::ios::binary);
#endif
		if (!file.good())
			return std::make_pair(nullptr, 0);

		auto length = get_file_length(file);
		auto data = new char[length + 1];
		file.read(data, length);
		data[length] = 0;
		return std::make_pair(std::unique_ptr<char[]>(data), length);
	}

	inline std::string get_file_string(const std::wstring &filename)
	{
		auto content = get_file_content(filename);
		if (content.first)
			return std::string(content.first.get(), content.first.get() + content.second);
		return std::string();
	}

	FLAME_FOUNDATION_EXPORTS void *get_hinst();
	FLAME_FOUNDATION_EXPORTS Vec2u get_screen_size();
	FLAME_FOUNDATION_EXPORTS const wchar_t *get_curr_path();
	FLAME_FOUNDATION_EXPORTS const wchar_t *get_app_path();
	FLAME_FOUNDATION_EXPORTS void com_init();
	FLAME_FOUNDATION_EXPORTS void read_process_memory(void *process, void *address, uint size, void *dst);
	FLAME_FOUNDATION_EXPORTS void sleep(uint time); // a time less than 0 means forever
	FLAME_FOUNDATION_EXPORTS void wait_for(void* ev);
	FLAME_FOUNDATION_EXPORTS void do_simple_dispatch_loop();
	FLAME_FOUNDATION_EXPORTS void exec(const std::wstring& filename, const std::wstring& parameters, bool wait, bool show = false);
	FLAME_FOUNDATION_EXPORTS String exec_and_get_output(const std::wstring& filename, const std::wstring& parameters);
	FLAME_FOUNDATION_EXPORTS String compile_to_dll(const std::vector<std::wstring>& sources, const std::vector<std::wstring>& libraries, const std::wstring& out);

	FLAME_FOUNDATION_EXPORTS DynamicArray<String> get_module_dependancies(const std::wstring& module_name);
	FLAME_FOUNDATION_EXPORTS void* load_module(const std::wstring& module_name);
	FLAME_FOUNDATION_EXPORTS void free_module(void* library);

	FLAME_FOUNDATION_EXPORTS StringW get_clipboard();
	FLAME_FOUNDATION_EXPORTS void set_clipboard(const std::wstring& s);

	FLAME_FOUNDATION_EXPORTS void open_explorer_and_select(const std::wstring& filename);
	FLAME_FOUNDATION_EXPORTS void move_to_trashbin(const std::wstring& filename);
	FLAME_FOUNDATION_EXPORTS void get_thumbnai(uint width, const std::wstring& filename, uint* out_width, uint* out_height, char **out_data);

	FLAME_FOUNDATION_EXPORTS Key vk_code_to_key(int vkCode);
	FLAME_FOUNDATION_EXPORTS bool is_modifier_pressing(Key k /* accept: Key_Shift, Key_Ctrl and Key_Alt */, int left_or_right /* 0 or 1 */);

	FLAME_FOUNDATION_EXPORTS void* add_global_key_listener(Key key, bool modifier_shift, bool modifier_ctrl, bool modifier_alt, const Function<void(void* c, KeyState action)>& callback);
	FLAME_FOUNDATION_EXPORTS void remove_global_key_listener(void *handle/* return by add_global_key_listener */);

	struct FileWatcher;
	typedef FileWatcher* FileWatcherPtr;

	enum FileWatcherOption
	{
		FileWatcherMonitorAllChanges = 1 << 0,
		FileWatcherMonitorOnlyContentChanged = 1 << 1,
		FileWatcherSynchronous = 1 << 2,
		FileWatcherAsynchronous = 1 << 3
	};

	enum FileChangeType
	{
		FileAdded,
		FileRemoved,
		FileModified,
		FileRenamed
	};

	FLAME_FOUNDATION_EXPORTS FileWatcher *add_file_watcher(const std::wstring& path, const Function<void(void* c, FileChangeType type, const std::wstring& filename)>& callback, int options = FileWatcherMonitorAllChanges | FileWatcherAsynchronous); // when you're using FileWatcherSynchronous, this func will not return untill something wrong, and return value is always nullptr
	FLAME_FOUNDATION_EXPORTS void remove_file_watcher(FileWatcher* w);

	FLAME_FOUNDATION_EXPORTS void add_work(const Function<void(void* c)>& fun);
	FLAME_FOUNDATION_EXPORTS void clear_works();
}
