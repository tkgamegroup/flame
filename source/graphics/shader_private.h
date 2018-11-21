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

#include <flame/graphics/shader.h>
#include "graphics_private.h"

#include <list>

namespace flame
{
	struct XmlNode;

	namespace graphics
	{
		struct DevicePrivate;

		struct ShaderPrivate : Shader
		{
			std::wstring filename_;
			std::string prefix_;
			std::vector<std::unique_ptr<ShaderResource>> resources;

			DevicePrivate *d;
			VkShaderModule v;

			int ref_count;

			ShaderPrivate(Device *d, const std::wstring &filename, const std::string &prefix);
			~ShaderPrivate();

			bool same(const std::wstring &filename, const std::string &prefix);
			ShaderResource *get_resource(const char *name);
			void load_members(XmlNode*, ShaderVariableType*);
		};
	}
}
