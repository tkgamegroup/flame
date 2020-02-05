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

	struct ClientSocket
	{
		FLAME_NETWORK_EXPORTS ClientSocket* create(SocketType type);
		FLAME_NETWORK_EXPORTS void destroy(ClientSocket* sock);
	};

	struct OneClientServer // only first one client can connect
	{
		void* ev_closed;

		FLAME_NETWORK_EXPORTS bool send(int size, void* data);

		FLAME_NETWORK_EXPORTS static OneClientServer* create(SocketType type, ushort port, int timeout/* second */, void on_message(void* c, const std::string& str), const Mail<>& capture);
		FLAME_NETWORK_EXPORTS static void destroy(OneClientServer* sock);
	};

	struct FrameSyncServer
	{
		void* ev_closed;

		FLAME_NETWORK_EXPORTS bool send(int client_idx, int size, void* data);

		FLAME_NETWORK_EXPORTS static FrameSyncServer* create(SocketType type, ushort port, int client_count);
		FLAME_NETWORK_EXPORTS static void destroy(FrameSyncServer* sock);
	};

	FLAME_NETWORK_EXPORTS void network_init();
}
