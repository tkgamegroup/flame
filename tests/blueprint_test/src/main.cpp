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

#include <flame/string.h>
#include <flame/blueprint.h>
#include <flame/typeinfo.h>
#include <flame/graphics/graphics.h>

using namespace flame;

int main(int argc, char **args)
{
	typeinfo::load(L"typeinfo.xml");

	//auto s = blueprint::Scene::create_from_file(L"d:/1.bp");
	auto s = blueprint::Scene::create();
	auto n_fmt = (blueprint::NodeInputEnumSingle*)s->add_node_input_enum_single(H("graphics::Format"), graphics::Format_Swapchain_B8G8R8A8_UNORM);
	auto n_sc = (blueprint::NodeInputEnumSingle*)s->add_node_input_enum_single(H("graphics::SampleCount"), graphics::SampleCount_8);
	auto n_att0 = (blueprint::NodeUDT*)s->add_node_udt(H("graphics::AttachmentInfo"));
	n_att0->insl(0)->item(0)->n = n_fmt;
	n_att0->insl(1)->item(0)->d.i[0] = 1;
	n_att0->insl(2)->item(0)->n = n_sc;
	auto n_att1 = (blueprint::NodeUDT*)s->add_node_udt(H("graphics::AttachmentInfo"));
	n_att1->insl(0)->item(0)->n = n_fmt;
	s->save(L"d:/1.xml");

	return 0;
}
