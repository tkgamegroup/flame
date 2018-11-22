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
#ifdef FLAME_SERIALIZE_MODULE
#define FLAME_SERIALIZE_EXPORTS __declspec(dllexport)
#else
#define FLAME_SERIALIZE_EXPORTS __declspec(dllimport)
#endif
#else
#define FLAME_SERIALIZE_EXPORTS
#endif

#include "math.h"
#include "string.h"

#include <stdio.h>

namespace flame
{
	struct XmlNode;

	namespace typeinfo
	{
		namespace cpp
		{
			struct UDT;
		}
	}

	inline String to_string(float v, int precision = 6)
	{
		char buf[20];
		sprintf(buf, "%.*f", precision, v);
		return buf;
	}

	inline String to_string(const Vec2 &v, int precision = 6)
	{
		char buf[40];
		sprintf(buf, "%.*f;%.*f", precision, v.x, precision, v.y);
		return buf;
	}

	inline String to_string(const Vec3 &v, int precision = 6)
	{
		char buf[60];
		sprintf(buf, "%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z);
		return buf;
	}

	inline String to_string(const Vec4 &v, int precision = 6)
	{
		char buf[80];
		sprintf(buf, "%.*f;%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z, precision, v.w);
		return buf;
	}

	inline String to_string(bool v)
	{
		return v ? "true" : "false";
	}

	inline String to_string(int v)
	{
		char buf[20];
		sprintf(buf, "%d", v);
		return buf;
	}

	inline String to_string(uint v)
	{
		char buf[20];
		sprintf(buf, "%d", v);
		return buf;
	}

	inline String to_string(const Ivec2 &v)
	{
		char buf[40];
		sprintf(buf, "%d;%d", v.x, v.y);
		return buf;
	}

	inline String to_string(const Ivec3 &v)
	{
		char buf[60];
		sprintf(buf, "%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline String to_string(const Ivec4 &v)
	{
		char buf[80];
		sprintf(buf, "%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline String to_string(const Bvec2 &v)
	{
		char buf[40];
		sprintf(buf, "%d;%d", v.x, v.y);
		return buf;
	}

	inline String to_string(const Bvec3 &v)
	{
		char buf[60];
		sprintf(buf, "%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline String to_string(const Bvec4 &v)
	{
		char buf[80];
		sprintf(buf, "%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline std::wstring to_wstring(float v, int precision = 6)
	{
		wchar_t buf[20];
		swprintf(buf, L"%.*f", precision, v);
		return buf;
	}

	inline StringW to_wstring(const Vec2 &v, int precision = 6)
	{
		wchar_t buf[40];
		swprintf(buf, L"%.*f;%.*f", precision, v.x, precision, v.y);
		return buf;
	}

	inline StringW to_wstring(const Vec3 &v, int precision = 6)
	{
		wchar_t buf[60];
		swprintf(buf, L"%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z);
		return buf;
	}

	inline StringW to_wstring(const Vec4 &v, int precision = 6)
	{
		wchar_t buf[80];
		swprintf(buf, L"%.*f;%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z, precision, v.w);
		return buf;
	}

	inline StringW to_wstring(bool v)
	{
		return v ? L"true" : L"false";
	}

	inline StringW to_wstring(int v)
	{
		wchar_t buf[20];
		swprintf(buf, L"%d", v);
		return buf;
	}

	inline StringW to_wstring(uint v)
	{
		wchar_t buf[20];
		swprintf(buf, L"%d", v);
		return buf;
	}

	inline StringW to_wstring(const Ivec2 &v)
	{
		wchar_t buf[40];
		swprintf(buf, L"%d;%d", v.x, v.y);
		return buf;
	}

	inline StringW to_wstring(const Ivec3 &v)
	{
		wchar_t buf[60];
		swprintf(buf, L"%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline StringW to_wstring(const Ivec4 &v)
	{
		wchar_t buf[80];
		swprintf(buf, L"%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline StringW to_wstring(const Bvec2 &v)
	{
		wchar_t buf[40];
		swprintf(buf, L"%d;%d", v.x, v.y);
		return buf;
	}

	inline StringW to_wstring(const Bvec3 &v)
	{
		wchar_t buf[60];
		swprintf(buf, L"%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline StringW to_wstring(const Bvec4 &v)
	{
		wchar_t buf[80];
		swprintf(buf, L"%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline std::string to_stdstring(float v, int precision = 6)
	{
		char buf[20];
		sprintf(buf, "%.*f", precision, v);
		return buf;
	}

	inline std::string to_stdstring(const Vec2 &v, int precision = 6)
	{
		char buf[40];
		sprintf(buf, "%.*f;%.*f", precision, v.x, precision, v.y);
		return buf;
	}

	inline std::string to_stdstring(const Vec3 &v, int precision = 6)
	{
		char buf[60];
		sprintf(buf, "%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z);
		return buf;
	}

	inline std::string to_stdstring(const Vec4 &v, int precision = 6)
	{
		char buf[80];
		sprintf(buf, "%.*f;%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z, precision, v.w);
		return buf;
	}

	inline std::string to_stdstring(bool v)
	{
		return v ? "true" : "false";
	}

	inline std::string to_stdstring(int v)
	{
		char buf[20];
		sprintf(buf, "%d", v);
		return buf;
	}

	inline std::string to_stdstring(uint v)
	{
		char buf[20];
		sprintf(buf, "%d", v);
		return buf;
	}

	inline std::string to_stdstring(const Ivec2 &v)
	{
		char buf[40];
		sprintf(buf, "%d;%d", v.x, v.y);
		return buf;
	}

	inline std::string to_stdstring(const Ivec3 &v)
	{
		char buf[60];
		sprintf(buf, "%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline std::string to_stdstring(const Ivec4 &v)
	{
		char buf[80];
		sprintf(buf, "%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline std::string to_stdstring(const Bvec2 &v)
	{
		char buf[40];
		sprintf(buf, "%d;%d", v.x, v.y);
		return buf;
	}

	inline std::string to_stdstring(const Bvec3 &v)
	{
		char buf[60];
		sprintf(buf, "%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline std::string to_stdstring(const Bvec4 &v)
	{
		char buf[80];
		sprintf(buf, "%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline std::wstring to_stdwstring(float v, int precision = 6)
	{
		wchar_t buf[20];
		swprintf(buf, L"%.*f", precision, v);
		return buf;
	}

	inline std::wstring to_stdwstring(const Vec2 &v, int precision = 6)
	{
		wchar_t buf[40];
		swprintf(buf, L"%.*f;%.*f", precision, v.x, precision, v.y);
		return buf;
	}

	inline std::wstring to_stdwstring(const Vec3 &v, int precision = 6)
	{
		wchar_t buf[60];
		swprintf(buf, L"%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z);
		return buf;
	}

	inline std::wstring to_stdwstring(const Vec4 &v, int precision = 6)
	{
		wchar_t buf[80];
		swprintf(buf, L"%.*f;%.*f;%.*f;%.*f", precision, v.x, precision, v.y, precision, v.z, precision, v.w);
		return buf;
	}

	inline std::wstring to_stdwstring(bool v)
	{
		return v ? L"true" : L"false";
	}

	inline std::wstring to_stdwstring(int v)
	{
		wchar_t buf[20];
		swprintf(buf, L"%d", v);
		return buf;
	}

	inline std::wstring to_stdwstring(uint v)
	{
		wchar_t buf[20];
		swprintf(buf, L"%d", v);
		return buf;
	}

	inline std::wstring to_stdwstring(const Ivec2 &v)
	{
		wchar_t buf[40];
		swprintf(buf, L"%d;%d", v.x, v.y);
		return buf;
	}

	inline std::wstring to_stdwstring(const Ivec3 &v)
	{
		wchar_t buf[60];
		swprintf(buf, L"%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline std::wstring to_stdwstring(const Ivec4 &v)
	{
		wchar_t buf[80];
		swprintf(buf, L"%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline std::wstring to_stdwstring(const Bvec2 &v)
	{
		wchar_t buf[40];
		swprintf(buf, L"%d;%d", v.x, v.y);
		return buf;
	}

	inline std::wstring to_stdwstring(const Bvec3 &v)
	{
		wchar_t buf[60];
		swprintf(buf, L"%d;%d;%d", v.x, v.y, v.z);
		return buf;
	}

	inline std::wstring to_stdwstring(const Bvec4 &v)
	{
		wchar_t buf[80];
		swprintf(buf, L"%d;%d;%d;%d", v.x, v.y, v.z, v.w);
		return buf;
	}

	inline float stof1(const std::string &s)
	{
		float ret;
		sscanf(s.c_str(), "%f", &ret);
		return ret;
	}

	inline Vec2 stof2(const std::string &s)
	{
		Vec2 ret;
		sscanf(s.c_str(), "%f;%f", &ret.x, &ret.y);
		return ret;
	}

	inline Vec3 stof3(const std::string &s)
	{
		Vec3 ret;
		sscanf(s.c_str(), "%f;%f;%f", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	inline Vec4 stof4(const std::string &s)
	{
		Vec4 ret;
		sscanf(s.c_str(), "%f;%f;%f;%f", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	inline int stoi1(const std::string &s)
	{
		int ret;
		sscanf(s.c_str(), "%d", &ret);
		return ret;
	}

	inline Ivec2 stoi2(const std::string &s)
	{
		Ivec2 ret;
		sscanf(s.c_str(), "%d;%d", &ret.x, &ret.y);
		return ret;
	}

	inline Ivec3 stoi3(const std::string &s)
	{
		Ivec3 ret;
		sscanf(s.c_str(), "%d;%d;%d", &ret.x, &ret.y, &ret.z);
		return ret;
	}

	inline Ivec4 stoi4(const std::string &s)
	{
		Ivec4 ret;
		sscanf(s.c_str(), "%d;%d;%d;%d", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	inline Bvec4 stob4(const std::string &s)
	{
		Bvec4 ret;
		sscanf(s.c_str(), "%d;%d;%d;%d", &ret.x, &ret.y, &ret.z, &ret.w);
		return ret;
	}

	FLAME_SERIALIZE_EXPORTS void serialize(XmlNode *n, typeinfo::cpp::UDT *u, void *obj, int precision = 6);
}

