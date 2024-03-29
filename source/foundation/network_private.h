#pragma once

#include "../json.h"
#include "network.h"

namespace flame
{
	namespace network
	{
		struct ClientPrivate : Client
		{
			SocketType type;

			int fd;

			std::function<void(const std::string& msg)> on_message;
			std::function<void()> on_close;

			std::recursive_mutex mtx;
			void* ev_ended;

			~ClientPrivate();

			void send(const std::string& msg) override;
			void stop(bool passive);
		};

		struct ServerPrivate : Server
		{
			struct Client
			{
				int fd;

				std::function<void(const std::string& msg)> on_message;
				std::function<void()> on_close;

				std::recursive_mutex mtx;
				void* ev_ended;

				~Client();

				void stop(bool passive);
			};

			SocketType type;

			int fd_d;
			int fd_s;

			std::vector<std::unique_ptr<Client>> cs;

			std::function<void(void* id, const std::string& msg)> on_dgram;
			std::function<void(void* id)> on_connect;

			std::recursive_mutex mtx;
			void* ev_ended_d;
			void* ev_ended_s;

			~ServerPrivate();

			void set_client(void* id, const std::function<void(const std::string& msg)>& on_message, const std::function<void()>& on_close) override;
			void send(void* id, const std::string& msg, bool dgram) override;
			void stop();
		};

		struct FrameSyncServerPrivate : FrameSyncServer
		{
			SocketType type;

			int frame;
			int semaphore;

			std::vector<int> fd_cs;

			nlohmann::json frame_data;

			void* ev_ended;

			~FrameSyncServerPrivate();

			bool send(uint idx, const std::string& msg) override;
			void stop();
		};
	}
}
