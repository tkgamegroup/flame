#pragma once

#ifdef FLAME_NETWORK_MODULE
#define FLAME_NETWORK_EXPORTS __declspec(dllexport)
#else
#define FLAME_NETWORK_EXPORTS __declspec(dllimport)
#endif

#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

namespace flame
{
	enum SocketType
	{
		SocketNormal,
		SocketWeb
	};

	struct Client
	{
		FLAME_NETWORK_EXPORTS void send(uint size, void* data);

		FLAME_NETWORK_EXPORTS static Client* create(SocketType type, const char* ip, uint port, void on_message(void* c, const char* msg), void on_close(void* c), const Mail<>& capture);
		FLAME_NETWORK_EXPORTS static void destroy(Client* c);
	};

	struct Server
	{
		FLAME_NETWORK_EXPORTS void set_client(void* id, void on_message(void* c, const char* msg), void on_close(void* c), const Mail<>& capture);
		FLAME_NETWORK_EXPORTS void send(void* id, uint size, void* data);

		FLAME_NETWORK_EXPORTS static Server* create(SocketType type, uint port, void on_connect(void* c, void* id), const Mail<>& capture);
		FLAME_NETWORK_EXPORTS static void destroy(Server* s);
	};

	struct OneClientServer // only first one client can connect
	{
		void* ev_closed;

		FLAME_NETWORK_EXPORTS bool send(uint size, void* data);

		FLAME_NETWORK_EXPORTS static OneClientServer* create(SocketType type, uint port, uint timeout/* second */, void on_message(void* c, const char* msg), const Mail<>& capture);
		FLAME_NETWORK_EXPORTS static void destroy(OneClientServer* s);
	};

	struct FrameSyncServer
	{
		void* ev_closed;

		FLAME_NETWORK_EXPORTS bool send(uint client_idx, uint size, void* data);

		FLAME_NETWORK_EXPORTS static FrameSyncServer* create(SocketType type, uint port, uint client_count);
		FLAME_NETWORK_EXPORTS static void destroy(FrameSyncServer* s);
	};
}
