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

namespace std
{
	namespace filesystem = std::experimental::filesystem;
}

namespace flame
{
	enum Err
	{
		NoErr,
		ErrInvalidEnum,
		ErrInvalidValue,
		ErrInvalidOperation,
		ErrOutOfMemory,
		ErrContextLost,
		ErrResourceLost
	};

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
		Key_PgUp,
		Key_PgDn,
		Key_End,
		Key_Home,
		Key_Left,
		Key_Up,
		Key_Right,
		Key_Down,
		Key_PrtSc,
		Key_Ins,
		Key_Del,
		Key_0,
		Key_1,
		Key_2,
		Key_3,
		Key_4,
		Key_5,
		Key_6,
		Key_7,
		Key_8,
		Key_9,
		Key_A,
		Key_B,
		Key_C,
		Key_D,
		Key_E,
		Key_F,
		Key_G,
		Key_H,
		Key_I,
		Key_J,
		Key_K,
		Key_L,
		Key_M,
		Key_N,
		Key_O,
		Key_P,
		Key_Q,
		Key_R,
		Key_S,
		Key_T,
		Key_U,
		Key_V,
		Key_W,
		Key_X,
		Key_Y,
		Key_Z,
		Key_Numpad0,
		Key_Numpad1,
		Key_Numpad2,
		Key_Numpad3,
		Key_Numpad4,
		Key_Numpad5,
		Key_Numpad6,
		Key_Numpad7,
		Key_Numpad8,
		Key_Numpad9,
		Key_Add,
		Key_Subtract,
		Key_Multiply,
		Key_Divide,
		Key_Separator,
		Key_Decimal,
		Key_F1,
		Key_F2,
		Key_F3,
		Key_F4,
		Key_F5,
		Key_F6,
		Key_F7,
		Key_F8,
		Key_F9,
		Key_F10,
		Key_F11,
		Key_F12,
		Key_NumLock,
		Key_ScrollLock,

		Key_count
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

	inline const char *get_error_string(Err errNum)
	{
		switch (errNum)
		{
		case NoErr:
			return "No error.";
		case ErrInvalidEnum:
			return "Invalid enum.";
		case ErrInvalidValue:
			return "Invalid value.";
		case ErrInvalidOperation:
			return "Invalid operation.";
		case ErrOutOfMemory:
			return "Out of memory.";
		case ErrContextLost:
			return "Context lost.";
		case ErrResourceLost:
			return "Resource lost.";
		default:
			return "Unknow error.";
		}
	}

	inline ulonglong get_now_ns()
	{
		return std::chrono::time_point_cast<std::chrono::nanoseconds>(
			std::chrono::system_clock::now()
			).time_since_epoch().count();
	}

	template<typename T>
	struct Array
	{
		int size;
		T *v;

		inline void resize(int new_size)
		{
			if (size == new_size)
				return;

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
				v = (T*)flame_realloc(v, sizeof(T) * new_size);

			if (new_size > size)
			{
				for (auto i = size; i < new_size; i++)
					new(&v[i])T();
			}

			size = new_size;
		}

		inline Array()
		{
			size = 0;
			v = nullptr;
		}

		inline Array(const Array<T> &rhs)
		{
			size = 0;
			v = nullptr;
			resize(rhs.size);
			memcpy(v, rhs.v, sizeof(T) * size);
		}

		inline ~Array()
		{
			flame_free(v);
		}

		inline void operator=(const Array<T> &rhs)
		{
			resize(rhs.size);
			memcpy(v, rhs.v, sizeof(T) * size);
		}

		inline T &operator[](int idx)
		{
			return v[idx];
		}

		inline const T &operator[](int idx) const
		{
			return v[idx];
		}

		inline void insert(int pos, const T &_v)
		{
			resize(size + 1);
			for (auto i = size - 1; i > pos; i--)
				memcpy(&v[i], &v[i - 1], sizeof(T));
			v[pos] = _v;
		}

		inline void push_back(const T &_v)
		{
			insert(size, _v);
		}

		inline void remove(int idx, int count = 1)
		{
			auto new_size = size - count;
			for (auto i = idx; i < new_size; i++)
				memcpy(&v[i], &v[i + count], sizeof(T));
			resize(new_size);
		}

		inline int find(const T &_v)
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

		inline void _assign(const CH *s, int len = 0)
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
			_assign(rhs.v, rhs.size);
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
			_assign(rhs.c_str(), rhs.size());
		}

		inline BasicString &operator=(const BasicString &rhs)
		{
			_assign(rhs.v, rhs.size);

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
			_assign(str);

			return *this;
		}

		inline BasicString &operator=(const std::basic_string<CH> &rhs)
		{
			_assign(rhs.c_str(), rhs.size());

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

	struct StringAndHash : String
	{
		uint hash;

		inline void _assign(const char *s, int len = 0)
		{
			String::_assign(s, len);
			hash = v ? H(v) : 0;
		}

		inline StringAndHash()
		{
			hash = 0;
		}

		inline StringAndHash(const StringAndHash &rhs) :
			StringAndHash()
		{
			_assign(rhs.v, rhs.size);
		}

		inline StringAndHash(StringAndHash &&rhs) :
			StringAndHash()
		{
			std::swap(size, rhs.size);
			std::swap(v, rhs.v);
			std::swap(hash, rhs.hash);
		}

		inline StringAndHash(const std::string &rhs) :
			StringAndHash()
		{
			_assign(rhs.c_str(), rhs.size());
		}

		inline StringAndHash &operator=(const StringAndHash &rhs)
		{
			_assign(rhs.v, rhs.size);

			return *this;
		}

		inline StringAndHash &operator=(StringAndHash &&rhs)
		{
			std::swap(size, rhs.size);
			std::swap(v, rhs.v);
			std::swap(hash, rhs.hash);

			return *this;
		}

		inline StringAndHash &operator=(const char *str)
		{
			_assign(str);

			return *this;
		}

		inline StringAndHash &operator=(const std::string &rhs)
		{
			_assign(rhs.c_str(), rhs.size());

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
	inline void string_to_lower$(std::basic_string<CH> &str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	}

	template<typename CH>
	inline void string_to_upper$(std::basic_string<CH> &str)
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

	inline void encode_base85(const uchar *src, int src_length, uchar *dst)
	{
		auto encode_char = [](uint v) {
			v = (v % 85) + 35;
			return (v >= ']') ? v + 1 : v;
		};

		for (auto i = 0; i < src_length; i += 4)
		{
			unsigned int d = *(uint*)(src + i);
			for (auto j = 0; j < 5; j++)
			{
				*dst = encode_char(d);
				dst++;
				d /= 85;
			}
		}
	}

	inline void decode_base85(const uchar *src, uchar *dst, int dst_length)
	{
		auto decode_char = [](uchar c) {
			return c >= ']' ? c - 36 : c - 35;
		};

		auto dst_off = 0;
		while (*src)
		{
			uint v = decode_char(src[0]) +
				85 * (decode_char(src[1]) +
					85 * (decode_char(src[2]) +
						85 * (decode_char(src[3]) +
							85 * decode_char(src[4]))));
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

	enum { MaxFunctionDataCount = 8 };

	struct Package
	{
		CommonData d[MaxFunctionDataCount];

		enum { SIZE = 0 };
		template<class CP/* capture package */>
		inline CP &get_capture()
		{
			return *(CP*)d; 
		}
	};

	typedef void(*PF)(Package &p);

	struct RegisteredFunction
	{
		uint id;
		PF pf;
		int parameter_count;
		String filename;
		int line_beg;
		int line_end;
	};

	FLAME_FOUNDATION_EXPORTS void register_function(uint id, PF pf, int parm_count, const char *filename, int line_beg, int line_end);
	FLAME_FOUNDATION_EXPORTS RegisteredFunction *find_registered_function(uint id, PF pf); // if !id, then use pf

#define FLAME_PACKAGE_BEGIN(name) \
	struct name : Package\
	{\
		enum { BASE = __COUNTER__ + 1 };
#define FLAME_PACKAGE_ITEM(t, n, tf) \
		inline t &n()\
		{\
			return (t&)d[__COUNTER__ - BASE].tf();\
		}
#define FLAME_PACKAGE_END \
		enum { SIZE = __COUNTER__ - BASE };\
		template<class CP/* capture package */>\
		inline CP &get_capture()\
		{\
			return *(CP*)(d + SIZE);\
		}\
	};

#define FLAME_REGISTER_FUNCTION_BEG(name, id, package) \
	struct name\
	{\
		name()\
		{\
			register_function(id, v, package::PARM_SIZE, __FILE__, line_beg, line_end);\
		}\
		static const int line_beg = __LINE__;\
		static void v(const ParmPackage &_p)\
		{\
			auto &p = (package&)_p;
#define FLAME_REGISTER_FUNCTION_END(name) \
		}\
		static const int line_end = __LINE__;\
	};\
	static name name##_;

	template<class PP = Package/* parameter package */>
	struct Function
	{
		typedef void(*TypedPF)(PP &p);
		TypedPF pf;
		PP p;
		int capture_count;

		inline Function()
		{
			pf = nullptr;
			capture_count = 0;
		}

		inline void set(TypedPF _pf, int parameter_count, const std::vector<CommonData> &capt)
		{
			pf = _pf;
			auto d = p.d + parameter_count;
			for (auto i = 0; i < capt.size(); i++)
			{
				*d = capt[i];
				d++;
			}
			capture_count = capt.size();
		}

		inline Function(TypedPF _pf, const std::vector<CommonData> &capt = {})
		{
			set(_pf, PP::SIZE, capt);
		}

		inline void exec()
		{
			pf(p);
		}
	};

	FLAME_FOUNDATION_EXPORTS void thread(Function<> &f);

	inline std::wstring ext_replace(const std::wstring &str, const std::wstring &ext)
	{
		std::filesystem::path path(str);
		if (path.extension().wstring() != ext)
		{
			auto pp = path.parent_path().wstring();
			if (pp != L"")
				pp += L"\\";
			return pp + path.stem().wstring() + ext;
		}
		return str;
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
		int size = 0;
		int q = 1;
		for (int i = 0; i < 4; i++)
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
		int size = v.size();
		for (int i = 0; i < 4; i++)
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
		return std::string(content.first.get(), content.first.get() + content.second);
	}

	FLAME_FOUNDATION_EXPORTS void *get_hinst();
	FLAME_FOUNDATION_EXPORTS Ivec2 get_screen_size();
	FLAME_FOUNDATION_EXPORTS const wchar_t *get_curr_path();
	FLAME_FOUNDATION_EXPORTS const wchar_t *get_app_path();
	FLAME_FOUNDATION_EXPORTS void read_process_memory(void *process, void *address, int size, void *dst);
	FLAME_FOUNDATION_EXPORTS void sleep(uint time); // a time less than 0 means forever
	FLAME_FOUNDATION_EXPORTS void do_simple_dispatch_loop();
	FLAME_FOUNDATION_EXPORTS void exec(const wchar_t *filename, const char *parameters, bool wait);
	FLAME_FOUNDATION_EXPORTS String exec_and_get_output(const wchar_t *filename, const char *parameters);

	// now, for void(void) member function or constructor only
	FLAME_FOUNDATION_EXPORTS void run_module_function(const wchar_t *module_name, const void *rva, void *thiz);

	FLAME_FOUNDATION_EXPORTS StringW get_clipboard();
	FLAME_FOUNDATION_EXPORTS void set_clipboard(const StringW &s);

	FLAME_FOUNDATION_EXPORTS void open_explorer_and_select(const wchar_t *filename);
	FLAME_FOUNDATION_EXPORTS void move_to_trashbin(const wchar_t *filename);
	FLAME_FOUNDATION_EXPORTS void get_thumbnai(int width, const wchar_t *filename, int *out_width, int *out_height, char **out_data);

	FLAME_FOUNDATION_EXPORTS Key vk_code_to_key(int vkCode);
	FLAME_FOUNDATION_EXPORTS bool is_modifier_pressing(Key k /* accept: Key_Shift, Key_Ctrl and Key_Alt */, int left_or_right /* 0 or 1 */);

	FLAME_PACKAGE_BEGIN(GlobalKeyParm)
		FLAME_PACKAGE_ITEM(KeyState, action, i1)
	FLAME_PACKAGE_END

	FLAME_FOUNDATION_EXPORTS void *add_global_key_listener(Key key, bool modifier_shift, bool modifier_ctrl, bool modifier_alt, Function<GlobalKeyParm> &callback);
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

	FLAME_PACKAGE_BEGIN(FileWatcherParm)
		FLAME_PACKAGE_ITEM(FileChangeType, type, i1)
		FLAME_PACKAGE_ITEM(wcharptr, filename, p)
	FLAME_PACKAGE_END

	FLAME_FOUNDATION_EXPORTS FileWatcher *add_file_watcher(const wchar_t *path, Function<FileWatcherParm> &callback, int options = FileWatcherMonitorAllChanges | FileWatcherAsynchronous); // when you're using FileWatcherSynchronous, this func will not return untill something wrong, and return value is always nullptr
	FLAME_FOUNDATION_EXPORTS void remove_file_watcher(FileWatcher *w);

	FLAME_FOUNDATION_EXPORTS void add_work(PF pf, char *capture_fmt, ...);
	FLAME_FOUNDATION_EXPORTS void clear_works();
}
