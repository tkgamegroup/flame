#pragma once

#include <flame/network/network.h>

namespace flame
{
	namespace network
	{
		struct ClientPrivate : Client
		{
			SocketType type;

			int fd;

			void (*on_message)(Capture& c, const char* msg, uint size);
			void (*on_close)(Capture& c);
			Capture capture;

			std::recursive_mutex mtx;
			void* ev_ended;

			~ClientPrivate();

			void release() override { delete this; }

			void send(void* data, uint size) override;
			void stop(bool passive);
		};

		struct ServerPrivate : Server
		{
			struct Client
			{
				int fd;

				void (*on_message)(Capture& c, const char* msg, uint size);
				void (*on_close)(Capture& c);
				Capture capture;

				std::recursive_mutex mtx;
				void* ev_ended;

				~Client();

				void stop(bool passive);
			};

			SocketType type;

			int fd_d;
			int fd_s;

			std::vector<std::unique_ptr<Client>> cs;

			void (*on_dgram)(Capture& c, void* id, const char* msg, uint size);
			void (*on_connect)(Capture& c, void* id);
			Capture capture;

			std::recursive_mutex mtx;
			void* ev_ended_d;
			void* ev_ended_s;

			~ServerPrivate();

			void release() override { delete this; }

			void set_client(void* id, void on_message(Capture& c, const char* msg, uint size), void on_close(Capture& c), const Capture& capture) override;
			void send(void* id, void* data, uint size, bool dgram) override;
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

			void release() override { delete this; }

			bool send(uint client_id, void* data, uint size) override;
			void stop();
		};
	}
}
