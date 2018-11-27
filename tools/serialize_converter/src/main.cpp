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

#include <flame/file.h>
#include <flame/serialize.h>

#include <stdio.h>

using namespace flame;

int main(int argc, char **args)
{
	char _filename[260];
	scanf("%s", _filename);

	auto filename = s2w(_filename);

	filesystem::path path(filename);
	auto dir = path.parent_path().string();
	if (path.extension() == ".xml")
	{
		auto file = SerializableNode::create_from_xml(filename);
		file->save_bin(ext_replace(filename, L"bin"));
		SerializableNode::destroy(file);
	}
	else /* if (path.extension() == ".bin") */
	{
		auto file = SerializableNode::create_from_bin(filename);
		file->save_xml(ext_replace(filename, L"xml"));
		SerializableNode::destroy(file);
	}

	return 0;
}
