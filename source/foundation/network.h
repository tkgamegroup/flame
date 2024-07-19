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
			SocketTcpRaw,
			SocketTcp,
			SocketWeb
		};

		struct Client
		{
			virtual ~Client() {}

			virtual void send(const std::string& msg) = 0;

			struct Create
			{
				virtual ClientPtr operator()(SocketType type, const char* ip, uint port, const std::function<void(const std::string& msg)>& on_message, const std::function<void()>& on_close) = 0;
			};
			FLAME_FOUNDATION_API static Create& create;
		};

		struct Server
		{
			virtual ~Server() {}

			virtual void set_client(void* id, const std::function<void(const std::string& msg)>& on_message, const std::function<void()>& on_close) = 0;
			virtual void send(void* id, const std::string& msg, bool dgram = false) = 0;

			struct Create
			{
				virtual ServerPtr operator()(SocketType type, uint port, const std::function<void(void* id, const std::string& msg)>& on_dgram, const std::function<void(void* id)>& on_connect) = 0;
			};
			FLAME_FOUNDATION_API static Create& create;
		};

		struct FrameSyncServer
		{
			virtual ~FrameSyncServer() {}

			virtual bool send(uint idx, const std::string& msg) = 0;

			struct Create
			{
				virtual FrameSyncServerPtr operator()(SocketType type, uint port, uint num_clients) = 0;
			};
			FLAME_FOUNDATION_API static Create& create;
		};

		// timeout: second
		FLAME_FOUNDATION_API void broadcast(uint port, const std::string& msg, uint timeout, const std::function<void(const char* ip, const std::string& msg)>& on_message);
		FLAME_FOUNDATION_API std::string download_html(const std::string& address);
	}
}
