// MIT License
// 
// Copyright (c) 2019 wjs
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

#include <flame/network/network.h>

using namespace flame;

struct Player
{
	int h_move;
	int v_move;
};

struct App
{
	FrameSyncServer* s;
	Player players[2];

	void create();
}app;
auto papp = &app;

void App::create()
{
	s = FrameSyncServer::create(SocketWeb, 5567, 2,
		Function<void(void* c, int client_idx, SerializableNode * src)>([](void* c, int client_idx, SerializableNode * src) {
				auto thiz = *(App * *)c;
				thiz->players[client_idx].h_move = stoi1(src->find_node("h_move")->value().c_str());
				thiz->players[client_idx].v_move = stoi1(src->find_node("v_move")->value().c_str());
			}, sizeof(void*), &papp),
		Function<void(void* c)>([](void* c) {
				auto thiz = *(App * *)c;
				auto json = SerializableNode::create("");
				json->new_attr("action", "frame");
				json->new_attr("h_move1", to_stdstring(thiz->players[0].h_move));
				json->new_attr("v_move1", to_stdstring(thiz->players[0].v_move));
				json->new_attr("h_move2", to_stdstring(thiz->players[1].h_move));
				json->new_attr("v_move2", to_stdstring(thiz->players[1].v_move));
				auto str = json->to_string_json();
				for (auto i = 0; i < 2; i++)
					thiz->s->send(i, str.size, str.v);
				SerializableNode::destroy(json);
			}, sizeof(void*), &papp));
}

int main(int argc, char **args)
{
	network_init();

	app.create();
	wait_for(app.s->ev_closed);

	return 0;
}
