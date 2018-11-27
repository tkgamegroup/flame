// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <flame/string.h>

#include <fstream>
#include <experimental/filesystem>
#include <stdarg.h>

namespace filesystem = std::experimental::filesystem;

namespace flame
{
	inline std::wstring ext_replace(const std::wstring &str, const std::wstring &ext)
	{
		filesystem::path path(str);
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
}
