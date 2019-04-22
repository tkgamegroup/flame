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

#ifdef FLAME_NETWORK_MODULE
#define FLAME_NETWORK_EXPORTS __declspec(dllexport)
#else
#define FLAME_NETWORK_EXPORTS __declspec(dllimport)
#endif

#include <flame/foundation/foundation.h>
#include <flame/foundation/serialize.h>

namespace flame
{
	enum SocketType
	{
		SocketNormal,
		SocketWeb
	};

	struct ClientSocket
	{
		FLAME_NETWORK_EXPORTS ClientSocket* create(SocketType type);
		FLAME_NETWORK_EXPORTS void destroy(ClientSocket* sock);
	};

	struct OneClientServer // only first one client can connect
	{
		void* ev_closed;

		FLAME_NETWORK_EXPORTS bool send(int size, void* data);

		FLAME_NETWORK_EXPORTS static OneClientServer* create(SocketType type, ushort port, int timeout/* second */, 
			const Function<void(void* c, int size, void* data)>& on_message);
		FLAME_NETWORK_EXPORTS static void destroy(OneClientServer* sock);
	};

	struct FrameSyncServer
	{
		void* ev_closed;

		FLAME_NETWORK_EXPORTS bool send(int client_idx, int size, void* data);

		FLAME_NETWORK_EXPORTS static FrameSyncServer* create(SocketType type, ushort port, int client_count, 
			const Function<void(void* c, int client_idx, SerializableNode* src)>& on_client_frame,
			const Function<void(void* c)>& on_frame_advance);
		FLAME_NETWORK_EXPORTS static void destroy(FrameSyncServer* sock);
	};

	FLAME_NETWORK_EXPORTS void network_init();
}
