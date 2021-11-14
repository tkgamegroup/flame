#pragma once

#include "foundation.h"

namespace flame
{
	namespace network
	{
		FLAME_FOUNDATION_TYPE(Client);
		FLAME_FOUNDATION_TYPE(Server);

		enum SocketType
		{
			SocketTcp,
			SocketWeb
		};

		struct Client
		{
			virtual void release() = 0;

			virtual void send(uint size, void* data) = 0;

			FLAME_FOUNDATION_EXPORTS static Client* create(SocketType type, const char* ip, uint port, void on_message(Capture& c, uint size, const char* msg), void on_close(Capture& c), const Capture& capture);
		};

		struct Server
		{
			virtual void release() = 0;

			virtual void set_client(void* id, void on_message(Capture& c, uint size, const char* msg), void on_close(Capture& c), const Capture& capture) = 0;
			virtual void send(void* id, uint size, void* data, bool dgram) = 0;

			FLAME_FOUNDATION_EXPORTS static Server* create(SocketType type, uint port, void on_dgram(Capture& c, void* id, uint size, const char* msg), void on_connect(Capture& c, void* id), const Capture& capture);
		};

		//struct FrameSyncServer
		//{
		//	virtual void release() = 0;

		//	virtual bool send(uint client_idx, uint size, void* data) = 0;

		//	FLAME_FOUNDATION_EXPORTS static FrameSyncServer* create(SocketType type, uint port, uint clients_count);
		//};

		FLAME_FOUNDATION_EXPORTS void board_cast(uint port, uint size, void* data, uint timeout/* second */, void on_message(Capture& c, const char* ip, uint size, const char* msg), const Capture& capture);
	}
}
