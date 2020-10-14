#pragma once

#ifdef FLAME_NETWORK_MODULE
#define FLAME_NETWORK_EXPORTS __declspec(dllexport)
#else
#define FLAME_NETWORK_EXPORTS __declspec(dllimport)
#endif

#include <flame/foundation/foundation.h>

namespace flame
{
	namespace network
	{
		FLAME_NETWORK_EXPORTS void initialize();

		enum SocketType
		{
			SocketNormal,
			SocketWeb
		};

		struct Client
		{
			virtual void release() = 0;

			virtual void send(void* data, uint size) = 0;

			FLAME_NETWORK_EXPORTS static Client* create(SocketType type, const char* ip, uint port, void on_message(Capture& c, const char* msg, uint size), void on_close(Capture& c), const Capture& capture);
		};

		struct Server
		{
			virtual void release() = 0;

			virtual void set_client(void* id, void on_message(Capture& c, const char* msg, uint size), void on_close(Capture& c), const Capture& capture) = 0;
			virtual void send(void* id, void* data, uint size, bool dgram) = 0;

			FLAME_NETWORK_EXPORTS static Server* create(SocketType type, uint port, void on_dgram(Capture& c, void* id, const char* msg, uint size), void on_connect(Capture& c, void* id), const Capture& capture);
		};

		struct FrameSyncServer
		{
			virtual void release() = 0;

			virtual bool send(uint client_idx, void* data, uint size) = 0;

			FLAME_NETWORK_EXPORTS static FrameSyncServer* create(SocketType type, uint port, uint clients_count);
		};

		FLAME_NETWORK_EXPORTS void board_cast(uint port, void* data, uint size, uint timeout/* second */, void on_message(Capture& c, const char* ip, const char* msg, uint size), const Capture& capture);
	}
}
