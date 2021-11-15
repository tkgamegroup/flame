#pragma once

#include "foundation.h"

namespace flame
{
	namespace network
	{
		FLAME_FOUNDATION_TYPE(Client);
		FLAME_FOUNDATION_TYPE(Server);
		FLAME_FOUNDATION_TYPE(FrameSyncServer);

		enum SocketType
		{
			SocketTcp,
			SocketWeb
		};

		struct Client
		{
			virtual ~Client() {}

			virtual void send(std::string_view msg) = 0;

			FLAME_FOUNDATION_EXPORTS static ClientPtr create(SocketType type, const char* ip, uint port, const std::function<void(std::string_view msg)>& on_message, const std::function<void()>& on_close);
		};

		struct Server
		{
			virtual ~Server() {}

			virtual void set_client(void* id, const std::function<void(std::string_view msg)>& on_message, const std::function<void()>& on_close) = 0;
			virtual void send(void* id, std::string_view msg, bool dgram) = 0;

			FLAME_FOUNDATION_EXPORTS static ServerPtr create(SocketType type, uint port, const std::function<void(void* id, std::string_view msg)>& on_dgram, const std::function<void(void* id)>& on_connect);
		};

		struct FrameSyncServer
		{
			virtual ~FrameSyncServer() {}

			virtual bool send(uint idx, std::string_view msg) = 0;

			FLAME_FOUNDATION_EXPORTS static FrameSyncServerPtr create(SocketType type, uint port, uint num_clients);
		};

		// timeout: second
		FLAME_FOUNDATION_EXPORTS void board_cast(uint port, std::string_view msg, uint timeout, const std::function<void(const char* ip, std::string_view msg)>& on_message);
	}
}
