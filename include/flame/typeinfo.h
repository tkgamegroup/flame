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

#ifdef FLAME_WINDOWS
#ifdef FLAME_TYPEINFO_MODULE
#define FLAME_TYPEINFO_EXPORTS __declspec(dllexport)
#else
#define FLAME_TYPEINFO_EXPORTS __declspec(dllimport)
#endif
#else
#define FLAME_TYPEINFO_EXPORTS
#endif

#include <flame/string.h>

namespace flame
{
	namespace typeinfo
	{
		namespace cpp
		{
			struct EnumItem
			{
				FLAME_TYPEINFO_EXPORTS const char *name() const;
				FLAME_TYPEINFO_EXPORTS int value() const;
			};

			struct EnumType
			{
				FLAME_TYPEINFO_EXPORTS const char *name() const;

				FLAME_TYPEINFO_EXPORTS int item_count() const;
				FLAME_TYPEINFO_EXPORTS EnumItem *item(int idx) const;
				FLAME_TYPEINFO_EXPORTS int find_item(const char *name) const;
				FLAME_TYPEINFO_EXPORTS int find_item(int value) const;

				FLAME_TYPEINFO_EXPORTS String serialize_value(bool single, int v) const;
			};

			enum VariableTag
			{
				VariableTagEnumSingle,
				VariableTagEnumMulti,
				VariableTagVariable,
				VariableTagPointer,
				VariableTagArrayOfVariable,
				VariableTagArrayOfPointer
			};

			struct VaribleInfo
			{
				FLAME_TYPEINFO_EXPORTS VariableTag tag() const;
				FLAME_TYPEINFO_EXPORTS const char *type_name() const;
				FLAME_TYPEINFO_EXPORTS uint type_hash() const;
				FLAME_TYPEINFO_EXPORTS const char *name() const;
				FLAME_TYPEINFO_EXPORTS int offset() const;

				FLAME_TYPEINFO_EXPORTS String serialize_value(void *src, bool is_obj, int precision = 6) const;
			};

			struct UDT
			{
				FLAME_TYPEINFO_EXPORTS const char *name() const;

				FLAME_TYPEINFO_EXPORTS int item_count() const;
				FLAME_TYPEINFO_EXPORTS VaribleInfo *item(int idx) const;
				FLAME_TYPEINFO_EXPORTS int find_item_i(const char *name) const;
			};

			FLAME_TYPEINFO_EXPORTS int enumeration_count();
			FLAME_TYPEINFO_EXPORTS EnumType *enumeration(int idx);
			FLAME_TYPEINFO_EXPORTS EnumType *find_enumeration(unsigned int name_hash);

			FLAME_TYPEINFO_EXPORTS int udt_count();
			FLAME_TYPEINFO_EXPORTS UDT *udt(int idx);
			FLAME_TYPEINFO_EXPORTS UDT *find_udt(unsigned int name_hash);
		}

		FLAME_TYPEINFO_EXPORTS int initialize_collecting();
		FLAME_TYPEINFO_EXPORTS void collect(const wchar_t *pdb_dir, const wchar_t *pdb_prefix);
		FLAME_TYPEINFO_EXPORTS void load(const wchar_t *filename);
		FLAME_TYPEINFO_EXPORTS void save(const wchar_t *filename);
		FLAME_TYPEINFO_EXPORTS void clear();
	}
}
